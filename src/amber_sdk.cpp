#include "amber_sdk.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <utility>
#include <wordexp.h>
#include <zlib.h>

#ifdef _WIN32
#include <Windows.h>
#else

#include <unistd.h>

#endif

using json = nlohmann::json;

const char *user_agent = "User-Agent: amber-cpp-sdk";

std::string compress_string(const std::string &str);

/**
 * Main client which interfaces with the Amber cloud. Amber account
 * credentials are discovered within a .Amber.license file located in the
 * home directory, or optionally overridden using environment variables.
 *
 * @param license_id: license identifier label found within .Amber.license file
 * @param license_file: path to .Amber.license file
 * @param verify_cert: whether to verify the ssl certificate or not
 * @param cert: file name of your client certificate
 * @param cainfo: a file holding one or more certificates to verify the peer
 * with
 *
 * Environment:
 *   AMBER_LICENSE_FILE: sets license_file path
 *
 *   AMBER_LICENSE_ID: sets license_id
 *
 *   AMBER_USERNAME: overrides the username as found in .Amber.license file
 *
 *   AMBER_PASSWORD: overrides the password as found in .Amber.license file
 *
 *   AMBER_SERVER: overrides the server as found in .Amber.license file
 *
 *   AMBER_SSL_CERT: path to SSL certificate
 *
 *   AMBER_SSL_VERIFY: Either a boolean, in which case it controls whether we
 * verify the server’s TLS certificate, or a string, in which case it must be a
 * path to a CA bundle to use
 *
 */
bool amber_sdk::amber_init(const std::string &l_id, const std::string &l_file,
                           bool verify_cert, const std::string &cert,
                           const std::string &cainfo) {
  this->auth_time = 0;
  this->expires_in = 0;
  this->auth_ok = false;

  // store license_file and license_id.  default values for these will be filled
  // in by constructors
  this->license_file = getenv("AMBER_LICENSE_FILE")
                           ? getenv("AMBER_LICENSE_FILE")
                           : std::string(l_file);
  this->license_id = getenv("AMBER_LICENSE_ID") ? getenv("AMBER_LICENSE_ID")
                                                : std::string(l_id);

  json amber_entry = {{"username", NULL},
                      {"password", NULL},
                      {"server", NULL},
                      {"oauth-server", NULL}};

  // create license profile
  if (!this->license_file.empty()) {

    // expand license file to full path
    wordexp_t exp_result;
    wordexp(this->license_file.c_str(), &exp_result, 0);
    this->license_file = exp_result.we_wordv[0];
    wordfree(&exp_result);

    // open up license file
    std::ifstream file(this->license_file.c_str());
    if (file.is_open()) {
      // load license file as json
      json license_json;
      try {
        file >> license_json;
      } catch (nlohmann::detail::parse_error &e) {
        throw amber_except("json not formatted correctly");
      }

      // locate license entry in license file
      if (!license_json.contains(this->license_id)) {
        throw amber_except("license_id '%s' not found in '%s'",
                           this->license_id.c_str(),
                           this->license_file.c_str());
      }

      // rewrite as license_entry structure
      amber_entry = license_json[this->license_id];
      try {
        this->license = amber_entry.get<amber_models::license_entry>();
      } catch (nlohmann::detail::out_of_range &e) {
        throw amber_except(e.what());
      }
    } else {
      // if license file is something other than default, throw exception
      if (strcmp(license_file.c_str(), "~/.Amber.license") != 0) {
        throw amber_except("license_file '%s' not found", license_file.c_str());
      }
    }
  }

  try {
    this->license.username = getenv("AMBER_USERNAME") ? getenv("AMBER_USERNAME")
                                                      : this->license.username;
    this->license.password = getenv("AMBER_PASSWORD") ? getenv("AMBER_PASSWORD")
                                                      : this->license.password;
    this->license.server =
        getenv("AMBER_SERVER") ? getenv("AMBER_SERVER") : this->license.server;
    this->license.oauth_server = getenv("AMBER_OAUTH_SERVER")
                                     ? getenv("AMBER_OAUTH_SERVER")
                                     : this->license.oauth_server;
    if (this->license.oauth_server.size() == 0) {
      this->license.oauth_server = this->license.server;
    }
    this->set_cert(getenv("AMBER_SSL_CERT") ? getenv("AMBER_SSL_CERT") : cert);

    this->verify_certificate(verify_cert);
    this->set_cainfo(cainfo);
    // verification override via env variable?
    if (getenv("AMBER_SSL_VERIFY")) {
      this->verify_certificate(true);
      if (strcasecmp("false", getenv("AMBER_SSL_VERIFY")) == 0) {
        this->verify_certificate(false);
        this->set_cainfo("");
      } else if (strcasecmp("true", getenv("AMBER_SSL_VERIFY")) != 0) {
        this->set_cainfo(getenv("AMBER_SSL_VERIFY"));
      }
    }
  } catch (json::exception &e) {
    throw amber_except("json failed with exception: %s", e.what());
  }

  if (this->license.username.compare("") == 0) {
    throw amber_except("username not specified");
  }
  if (this->license.password.compare("") == 0) {
    throw amber_except("password not specified");
  }
  if (this->license.server.compare("") == 0) {
    throw amber_except("server not specified");
  }
  return true;
}

amber_sdk::~amber_sdk() = default;

static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                            void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

error_response *amber_sdk::create_sensor(create_sensor_response &response,
                                         std::string label) {

  // generate sdk request object
  amber_models::PostSensorRequest request{std::move(label)};
  json j = request;
  auto sdk_req = sdk_request{"POST", "sensor"};
  sdk_req.body = j.dump();
  sdk_req.headers["content-type"] = "application/json";

  // call api and process results
  sdk_response sdk_res;
  this->call_api(sdk_req, sdk_res);
  if (sdk_res.code != 200) {
    return new error_response(sdk_res.res.get<amber_models::Error>());
  }
  response = sdk_res.res.get<create_sensor_response>();
  return nullptr;
}

error_response *amber_sdk::configure_sensor(
    configure_sensor_response &response, const std::string &sensor_id,
    uint16_t feature_count, uint16_t streaming_window_size,
    uint32_t samples_to_buffer, uint64_t learning_rate_numerator,
    uint64_t learning_rate_denominator, uint16_t learning_max_clusters,
    uint64_t learning_max_samples, uint32_t anomaly_history_window,
    std::vector<amber_models::FeatureConfig> features) {

  // generate sdk request object
  amber_models::PostConfigRequest request{
      anomaly_history_window,    learning_rate_numerator,
      learning_rate_denominator, learning_max_clusters,
      learning_max_samples,      feature_count,
      streaming_window_size,     std::move(features),
      samples_to_buffer};
  json j = request;
  auto sdk_req = sdk_request{"POST", "config"};
  sdk_req.body = j.dump();
  sdk_req.headers["content-type"] = "application/json";
  sdk_req.headers["sensorid"] = sensor_id;

  // call api and process results
  sdk_response sdk_res;
  this->call_api(sdk_req, sdk_res);
  if (sdk_res.code != 200) {
    return new error_response(sdk_res.res.get<amber_models::Error>());
  }
  response = sdk_res.res.get<configure_sensor_response>();
  return nullptr;
}

error_response *
amber_sdk::configure_fusion(configure_fusion_response &response,
                            const std::string &sensor_id,
                            const amber_models::PutConfigRequest &request) {

  // generate sdk request object
  json j = request;
  auto sdk_req = sdk_request{"PUT", "config"};
  sdk_req.body = j.dump();
  sdk_req.headers["content-type"] = "application/json";
  sdk_req.headers["sensorid"] = sensor_id;

  // call api and process results
  sdk_response sdk_res;
  this->call_api(sdk_req, sdk_res);
  if (sdk_res.code != 200) {
    return new error_response(sdk_res.res.get<amber_models::Error>());
  }
  response = sdk_res.res.get<configure_fusion_response>();
  return nullptr;
}

error_response *
amber_sdk::stream_fusion(stream_fusion_response &response,
                         const std::string &sensor_id,
                         const amber_models::PutStreamRequest &request) {

  // generate sdk request object
  json j = request;
  auto sdk_req = sdk_request{"PUT", "stream"};
  sdk_req.body = j.dump();
  sdk_req.headers["content-type"] = "application/json";
  sdk_req.headers["sensorid"] = sensor_id;

  // call api and process results
  sdk_response sdk_res;
  this->call_api(sdk_req, sdk_res);
  if (sdk_res.code != 200) {
    return new error_response(sdk_res.res.get<amber_models::Error>());
  }
  response = sdk_res.res.get<stream_fusion_response>();
  return nullptr;
}

error_response *amber_sdk::get_sensor(get_sensor_response &response,
                                      const std::string &sensor_id) {

  // generate sdk request object
  auto sdk_req = sdk_request{"GET", "sensor"};
  sdk_req.headers["content-type"] = "application/json";
  sdk_req.headers["sensorid"] = sensor_id;

  // call api and process results
  sdk_response sdk_res;
  this->call_api(sdk_req, sdk_res);
  if (sdk_res.code != 200) {
    return new error_response(sdk_res.res.get<amber_models::Error>());
  }
  response = sdk_res.res.get<get_sensor_response>();
  return nullptr;
}

error_response *amber_sdk::list_sensors(list_sensors_response &response) {

  // generate sdk request object
  auto sdk_req = sdk_request{"GET", "sensors"};
  sdk_req.headers["content-type"] = "application/json";

  // call api and process results
  sdk_response sdk_res;
  this->call_api(sdk_req, sdk_res);
  if (sdk_res.code != 200) {
    return new error_response(sdk_res.res.get<amber_models::Error>());
  }
  response = sdk_res.res.get<list_sensors_response>();
  return nullptr;
}

error_response *amber_sdk::update_sensor(update_sensor_response &response,
                                         const std::string sensor_id,
                                         std::string &label) {

  // generate sdk request object
  amber_models::PutSensorRequest request{label};
  json j = request;
  auto sdk_req = sdk_request{"PUT", "sensor"};
  sdk_req.body = j.dump();
  sdk_req.headers["content-type"] = "application/json";
  sdk_req.headers["sensorid"] = sensor_id;

  // call api and process results
  sdk_response sdk_res;
  this->call_api(sdk_req, sdk_res);
  if (sdk_res.code != 200) {
    return new error_response(sdk_res.res.get<amber_models::Error>());
  }
  response = sdk_res.res.get<update_sensor_response>();
  return nullptr;
}

error_response *amber_sdk::delete_sensor(delete_sensor_response &response,
                                         const std::string &sensor_id) {

  // generate sdk request object
  auto sdk_req = sdk_request{"DELETE", "sensor"};
  sdk_req.headers["content-type"] = "application/json";
  sdk_req.headers["sensorid"] = sensor_id;

  // call api and process results
  sdk_response sdk_res;
  this->call_api(sdk_req, sdk_res);
  if (sdk_res.code != 200) {
    return new error_response(sdk_res.res.get<error_response>());
  }
  response = sdk_res.res.get<error_response>();
  return nullptr;
}

error_response *amber_sdk::stream_sensor(stream_sensor_response &response,
                                         const std::string &sensor_id,
                                         std::string csvdata, bool save_image) {

  // generate sdk request object
  amber_models::PostStreamRequest request{save_image, std::move(csvdata)};
  json j = request;
  auto sdk_req = sdk_request{"POST", "stream"};
  sdk_req.body = j.dump();
  sdk_req.headers["content-type"] = "application/json";
  sdk_req.headers["sensorid"] = sensor_id;

  // call api and process results
  sdk_response sdk_res;
  this->call_api(sdk_req, sdk_res);
  if (sdk_res.code != 200) {
    return new error_response(sdk_res.res.get<error_response>());
  }
  response = sdk_res.res.get<stream_sensor_response>();
  return nullptr;
}

error_response *amber_sdk::enable_learning(enable_learning_response &response,
                                           const std::string &sensor_id,
                                           uint32_t anomaly_history_window,
                                           uint64_t learning_rate_numerator,
                                           uint64_t learning_rate_denominator,
                                           uint16_t learning_max_clusters,
                                           uint64_t learning_max_samples) {

  // generate sdk request object
  amber_models::LearningParameters streaming{
      learning_rate_numerator, learning_rate_denominator, learning_max_clusters,
      learning_max_samples};
  std::vector<amber_models::FusionConfig> fusion_config;
  amber_models::PutConfigRequest request{fusion_config, streaming};
  json j = request;
  j.erase("features");
  auto sdk_req = sdk_request{"PUT", "config"};
  sdk_req.body = j.dump();
  sdk_req.headers["content-type"] = "application/json";
  sdk_req.headers["sensorid"] = sensor_id;

  // call api and process results
  sdk_response sdk_res;
  this->call_api(sdk_req, sdk_res);
  if (sdk_res.code != 200) {
    return new error_response(sdk_res.res.get<error_response>());
  }
  response = sdk_res.res.get<enable_learning_response>();
  return nullptr;
}

error_response *amber_sdk::post_outage(post_outage_response &response,
                                       const std::string &sensor_id) {

  // generate sdk request object
  auto sdk_req = sdk_request{"POST", "outage"};
  sdk_req.headers["content-type"] = "application/json";
  sdk_req.headers["sensorid"] = sensor_id;

  // call api and process results
  sdk_response sdk_res;
  this->call_api(sdk_req, sdk_res);
  if (sdk_res.code != 200) {
    return new error_response(sdk_res.res.get<error_response>());
  }
  response = sdk_res.res.get<post_outage_response>();
  return nullptr;
}

error_response *
amber_sdk::pretrain_sensor_xl(pretrain_sensor_response &response,
                              const std::string &sensor_id, std::string csvdata,
                              bool autotuneConfig, bool blocking) {

  // parse csv data into float vector
  std::vector<float> packed_floats;
  std::stringstream ss(csvdata);
  for (float i; ss >> i;) {
    packed_floats.push_back(i);
    if (ss.peek() == ',' || isspace(ss.peek()))
      ss.ignore();
  }

  // compute the number of chunks needed to send all of the pretrain data
  size_t chunk_size = 1000000;
  size_t chunk_max = packed_floats.size() / chunk_size;
  if (packed_floats.size() % chunk_size != 0) {
    chunk_max++;
  }

  // send pretrain chunks
  std::string amber_transaction;
  for (int chunk_idx = 0; chunk_idx < chunk_max; chunk_idx++) {

    // calculate start and end positions
    size_t start = chunk_idx * chunk_size;
    size_t end = (chunk_idx + 1) * chunk_size;
    if (end > packed_floats.size()) {
      end = packed_floats.size();
    }

    // create next packed_float chunk
    std::vector<float>::const_iterator first =
        packed_floats.begin() + int(start);
    std::vector<float>::const_iterator last = packed_floats.begin() + int(end);
    std::vector<float> chunk(first, last);

    std::string encoded_chunk =
        base64_encode((unsigned char *)chunk.data(), chunk.size() * 4);
    amber_models::PostPretrainRequest request{encoded_chunk, "packed-float",
                                              autotuneConfig};
    json j = request;
    auto sdk_req = sdk_request{"POST", "pretrain"};
    sdk_req.body = j.dump();
    sdk_req.headers["content-type"] = "application/json";
    sdk_req.headers["sensorid"] = sensor_id;
    sdk_req.headers["amberchunk"] =
        std::to_string(chunk_idx + 1) + ":" + std::to_string(chunk_max);
    if (!amber_transaction.empty()) {
      sdk_req.headers["ambertransaction"] = amber_transaction;
    }

    // call api and process results
    sdk_response sdk_res;
    this->call_api(sdk_req, sdk_res);
    switch (sdk_res.code) {
    case 200:
      if (start != end) {
        return new error_response(sdk_res.res.get<error_response>());
      }
      response = sdk_res.res.get<pretrain_sensor_response>();
      break;
    case 202:
      amber_transaction = sdk_res.headers["ambertransaction"];
      response = sdk_res.res.get<pretrain_sensor_response>();
      break;
    default:
      return new error_response(sdk_res.res.get<error_response>());
    }
  }

  while (blocking && response.state == "Pretraining") {
    sleep(5);
    get_pretrain_response get_response;
    auto err = this->get_pretrain(get_response, sensor_id);
    if (err != nullptr) {
      return err;
    }
    response.amberChunk = "";
    response.amberTransaction = "";
    response.message = get_response.message;
    response.state = get_response.state;
  }

  return nullptr;
}

error_response *amber_sdk::pretrain_sensor(pretrain_sensor_response &response,
                                           const std::string &sensor_id,
                                           std::string csvdata,
                                           bool autotuneConfig, bool blocking) {

  // generate sdk request object
  amber_models::PostPretrainRequest request{csvdata, "csv", autotuneConfig};
  json j = request;
  auto sdk_req = sdk_request{"POST", "pretrain"};
  sdk_req.body = j.dump();
  sdk_req.headers["content-type"] = "application/json";
  sdk_req.headers["sensorid"] = sensor_id;

  // call api and process results
  sdk_response sdk_res;
  this->call_api(sdk_req, sdk_res);
  switch (sdk_res.code) {
  case 200:
    response = sdk_res.res.get<pretrain_sensor_response>();
    break;
  case 202: {
    get_pretrain_response get_response;
    get_response.state = "Pretraining";
    while (blocking && get_response.state == "Pretraining") {
      sleep(5);
      auto err = this->get_pretrain(get_response, sensor_id);
      if (err != nullptr) {
        return err;
      }
      response.message = get_response.message;
      response.state = get_response.state;
    }
  } break;
  default:
    return new error_response(sdk_res.res.get<error_response>());
  }
  return nullptr;
}

error_response *amber_sdk::get_pretrain(get_pretrain_response &response,
                                        const std::string &sensor_id) {

  // generate sdk request object
  auto sdk_req = sdk_request{"GET", "pretrain"};
  sdk_req.headers["content-type"] = "application/json";
  sdk_req.headers["sensorid"] = sensor_id;

  // call api and process results
  sdk_response sdk_res;
  this->call_api(sdk_req, sdk_res);
  if (sdk_res.code != 200 && sdk_res.code != 202) {
    return new error_response(sdk_res.res.get<error_response>());
  }
  response = sdk_res.res.get<get_pretrain_response>();
  return nullptr;
}

error_response *amber_sdk::get_config(amber_models::GetConfigResponse &response,
                                      const std::string &sensor_id) {

  // generate sdk request object
  auto sdk_req = sdk_request{"GET", "config"};
  sdk_req.headers["content-type"] = "application/json";
  sdk_req.headers["sensorid"] = sensor_id;

  // call api and process results
  sdk_response sdk_res;
  this->call_api(sdk_req, sdk_res);
  if (sdk_res.code != 200) {
    return new error_response(sdk_res.res.get<error_response>());
  }
  response = sdk_res.res.get<get_config_response>();
  return nullptr;
}

error_response *amber_sdk::get_version(get_version_response &response) {
  // generate sdk request object
  auto sdk_req = sdk_request{"GET", "version"};
  sdk_req.headers["content-type"] = "application/json";

  // call api and process results
  sdk_response sdk_res;
  this->call_api(sdk_req, sdk_res);
  if (sdk_res.code != 200) {
    return new error_response(sdk_res.res.get<error_response>());
  }
  response = sdk_res.res.get<get_version_response>();
  return nullptr;
}

error_response *amber_sdk::get_status(get_status_response &response,
                                      const std::string &sensor_id) {
  // generate sdk request object
  auto sdk_req = sdk_request{"GET", "status"};
  sdk_req.headers["content-type"] = "application/json";
  sdk_req.headers["sensorid"] = sensor_id;

  // call api and process results
  sdk_response sdk_res;
  this->call_api(sdk_req, sdk_res);
  if (sdk_res.code != 200) {
    return new error_response(sdk_res.res.get<error_response>());
  }
  response = sdk_res.res.get<get_status_response>();
  return nullptr;
}

error_response *
amber_sdk::get_root_cause_by_idlist(get_root_cause_response &response,
                                    const std::string &sensor_id,
                                    std::vector<uint16_t> idlist) {

  // generate sdk request object
  auto sdk_req = sdk_request{"GET", "rootCause"};
  sdk_req.headers["content-type"] = "application/json";
  sdk_req.headers["sensorid"] = sensor_id;
  sdk_req.query_params = "?clusterID=" + join_uint16_vec(idlist);

  // call api and process results
  sdk_response sdk_res;
  this->call_api(sdk_req, sdk_res);
  if (sdk_res.code != 200) {
    return new error_response(sdk_res.res.get<error_response>());
  }
  response = sdk_res.res.get<get_root_cause_response>();
  return nullptr;
}

error_response *amber_sdk::get_root_cause_by_patternlist(
    amber_models::GetRootCauseResponse &response, const std::string &sensor_id,
    std::vector<std::vector<uint16_t>> pattern_list) {

  // generate sdk request object
  auto sdk_req = sdk_request{"GET", "rootCause"};
  sdk_req.headers["content-type"] = "application/json";
  sdk_req.headers["sensorid"] = sensor_id;
  sdk_req.query_params = "?pattern=" + join_vec_uint16_vec(pattern_list);

  // call api and process results
  sdk_response sdk_res;
  this->call_api(sdk_req, sdk_res);
  if (sdk_res.code != 200) {
    return new error_response(sdk_res.res.get<error_response>());
  }
  response = sdk_res.res.get<get_root_cause_response>();
  return nullptr;
}

void amber_sdk::call_api(sdk_request &req, sdk_response &res, bool is_auth) {

  // set up curl request
  CURL *curl;
  curl = curl_easy_init();
  struct curl_slist *hs = nullptr;
  std::string url;

  res.code = 0;

  // authenticate
  if (is_auth) {
    // this call is performing authentication so access oauth server
    url = this->license.oauth_server + '/' + req.slug + req.query_params;
  } else {
    if (!this->authenticate(res)) {
      curl_easy_cleanup(curl);
      return;
    }
    url = this->license.server + '/' + req.slug + req.query_params;
    hs = curl_slist_append(hs, this->auth_bear_header.c_str());
  }

  // apply user_agent to identify the sdk in requests
  hs = curl_slist_append(hs, user_agent);

  // apply specified http headers
  for (auto &iter : req.headers) {
    auto header = iter.first + ':' + iter.second;
    hs = curl_slist_append(hs, header.c_str());
  }

  // apply operation
  if (req.operation == "POST") {
    if (req.body.size() > 10000) {
      req.body = compress_string(req.body);
      hs = curl_slist_append(hs, "Content-Encoding: gzip");
    }
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req.body.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, req.body.size());
  } else if (req.operation == "PUT") {
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req.body.c_str());
  } else if (req.operation == "GET") {
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
  } else if (req.operation == "DELETE") {
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
  } else {
    // bad operation
  }

  std::string read_buffer;   // response body
  std::string header_buffer; // response headers
  char error_buffer[CURL_ERROR_SIZE];
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hs);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, this->ssl.verify ? 1 : 0);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, this->ssl.verify ? 1 : 0);
  curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_buffer);
  if (this->ssl.verify) {
    if (!this->ssl.cert.empty()) {
      curl_easy_setopt(curl, CURLOPT_SSLCERT, this->ssl.cert.c_str());
    }
    if (!this->ssl.cainfo.empty()) {
      curl_easy_setopt(curl, CURLOPT_CAINFO, this->ssl.cainfo.c_str());
    }
  }

  // send request and process result
  auto curl_result = curl_easy_perform(curl);
  if (curl_result == CURLE_OK) {
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res.code);
    res.headers = parse_headers(header_buffer);
    res.res = json::parse(read_buffer);
  }
  curl_easy_cleanup(curl);
  curl_slist_free_all(hs);
}

/**
 * Authenticate client for the next hour using the credentials given at
  initialization. This acquires and stores an oauth2 token which remains
  valid for one hour and is used to authenticate all other API requests.
 * @return
 */
bool amber_sdk::authenticate(sdk_response &res) {

  if (this->auth_ok &&
      std::time(nullptr) + this->expires_in - 100 < this->auth_time) {
    // auth token is still good
    res.code = 200;
    return true;
  }
  this->auth_ok = false;

  // create request body
  auto request = amber_models::PostAuth2Request{this->license.username,
                                                this->license.password};

  // generate sdk request object
  json j = request;
  auto sdk_req = sdk_request{"POST", "oauth2"};
  sdk_req.body = j.dump();
  sdk_req.headers["content-type"] = "application/json";

  // call api with auth disabled (since this is the oauth2 call itself)
  sdk_response sdk_res;
  this->call_api(sdk_req, sdk_res, true);
  if (sdk_res.code != 200) {
    this->auth_ok = false;
    return false;
  }

  // process response and stash token
  this->auth = sdk_res.res;
  this->auth_ok = true;
  this->expires_in = std::stoul(this->auth.expiresIn, nullptr, 0);
  this->auth_time = std::time(nullptr) + this->expires_in;
  this->auth_bear_header =
      std::string("Authorization: Bearer " + this->auth.idToken);
  return true;
}

std::string compress_string(const std::string &str) {
  z_stream zs;
  memset(&zs, 0, sizeof(zs));

  // gzip init
  if (deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 | 16, 8,
                   Z_DEFAULT_STRATEGY) != Z_OK) {
    throw(std::runtime_error("deflateInit failed while compressing."));
  }

  zs.next_in = (Bytef *)str.data();
  zs.avail_in = str.size(); // set the z_stream's input

  int ret;
  char outbuffer[32768];
  std::string outstring;

  // retrieve the compressed bytes blockwise
  do {
    zs.next_out = reinterpret_cast<Bytef *>(outbuffer);
    zs.avail_out = sizeof(outbuffer);

    ret = deflate(&zs, Z_FINISH);

    if (outstring.size() < zs.total_out) {
      // append the block to the output string
      outstring.append(outbuffer, zs.total_out - outstring.size());
    }
  } while (ret == Z_OK);

  deflateEnd(&zs);

  if (ret != Z_STREAM_END) { // an error occurred that was not EOF
    std::ostringstream oss;
    oss << "Exception during zlib compression: (" << ret << ") " << zs.msg;
    throw(std::runtime_error(oss.str()));
  }

  return outstring;
}

static std::string base64_encode(const unsigned char *in, uint64_t len) {

  std::string out;

  int val = 0, valb = -6;
  for (auto idx = 0; idx < len; idx++) {
    auto c = *(in + idx);
    val = (val << 8) + c;
    valb += 8;
    while (valb >= 0) {
      out.push_back(
          "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
              [(val >> valb) & 0x3F]);
      valb -= 6;
    }
  }
  if (valb > -6)
    out.push_back(
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
            [((val << 8) >> (valb + 8)) & 0x3F]);
  while (out.size() % 4)
    out.push_back('=');
  return out;
}

std::string &ltrim(std::string &s) {
  auto it = std::find_if(s.begin(), s.end(), [](char c) {
    return !std::isspace<char>(c, std::locale::classic());
  });
  s.erase(s.begin(), it);
  return s;
}

std::string &rtrim(std::string &s) {
  auto it = std::find_if(s.rbegin(), s.rend(), [](char c) {
    return !std::isspace<char>(c, std::locale::classic());
  });
  s.erase(it.base(), s.end());
  return s;
}

std::string &trim(std::string &s) { return ltrim(rtrim(s)); }

static std::map<std::string, std::string>
parse_headers(const std::string &header) {
  std::map<std::string, std::string> header_map;
  std::string delimiter = "\r\n";
  unsigned long copy_cnt;
  for (size_t i = 0; i < header.size();) {
    auto del = header.find(delimiter, i);
    if (del == std::string::npos) {
      copy_cnt = header.size() - i;
    } else {
      copy_cnt = del - i;
    }

    // extract next k=v
    auto next_header = header.substr(i, copy_cnt);

    // apply k:v to map
    auto kvdel = next_header.find(':', 0);
    if (kvdel != std::string::npos) {
      auto k = next_header.substr(0, kvdel);
      auto v = next_header.substr(kvdel + 1);
      header_map[trim(k)] = trim(v);
    }

    // advance index
    if (del != std::string::npos) {
      // advance past crnl token
      i = del + delimiter.size();
    } else {
      i = header.size();
    }
  }
  return header_map;
}

std::string join_uint16_vec(std::vector<uint16_t> const &v) {
  std::string id_string;
  for (uint16_t i : v) {
    if (id_string.empty()) {
      id_string = "[" + std::to_string(i);
    } else {
      id_string += "," + std::to_string(i);
    }
  }
  id_string += "]";
  return id_string;
}

std::string join_vec_uint16_vec(std::vector<std::vector<uint16_t>> const &v) {
  std::string id_string;

  for (std::vector<uint16_t> l : v) {
    auto next_vec = join_uint16_vec(l);
    if (id_string.empty()) {
      id_string = "[" + next_vec;
    } else {
      id_string += "," + next_vec;
    }
  }
  id_string += "]";
  return id_string;
}
