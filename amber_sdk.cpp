#include "amber_sdk.h"
#include <fstream>
#include <sstream>
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
amber_sdk::amber_sdk(const char *license_id, const char *license_file,
                     bool verify_cert, const char *cert, const char *cainfo) {
  this->auth_time = 0;
  this->last_code = 0;
  this->last_error[0] = '\0';
  this->expires_in = 0;
  this->auth_ok = false;

  if (license_id == NULL) {
    license_id = "";
  }
  if (license_file == NULL) {
    license_file = "";
  }
  if (cert == NULL) {
    cert = "";
  }
  if (cainfo == NULL) {
    cainfo = "";
  }

  // first load license file
  this->license_file = getenv("AMBER_LICENSE_FILE")
                           ? getenv("AMBER_LICENSE_FILE")
                           : std::string(license_file);

  // next get license ID
  this->license_id = getenv("AMBER_LICENSE_ID") ? getenv("AMBER_LICENSE_ID")
                                                : std::string(license_id);

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
    std::ifstream file(this->license_file);
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
      if (strcmp(license_file, "~/.Amber.license") != 0) {
        throw amber_except("license_file '%s' not found", license_file);
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
    if (this->license.oauth_server.length() == 0) {
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
}

amber_sdk::~amber_sdk() = default;

static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                            void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

bool amber_sdk::create_sensor(amber_models::create_sensor_response &response,
                              std::string *label) {
  std::string defaultLabel = std::string();
  if (label) {
    defaultLabel = *label;
  }
  amber_models::create_sensor_request request(defaultLabel);
  json j = request;
  std::string body = j.dump();
  json json_response;
  std::string no_sensor;
  std::string slug = "/sensor";
  std::string url = this->license.server + slug;
  try {
    if (this->post_request(url, no_sensor, body, true, json_response) != 200) {
      return false;
    }
    response = json_response.get<amber_models::create_sensor_response>();
    return true;
  } catch (const std::exception &e) {
    fprintf(stderr, "%s\n", e.what());
    return false;
  }
}

bool amber_sdk::configure_sensor(
    amber_models::configure_sensor_response &response,
    const std::string &sensor_id, uint16_t feature_count,
    uint16_t streaming_window_size, uint32_t samples_to_buffer,
    uint64_t learning_rate_numerator, uint32_t learning_rate_denominator,
    uint16_t learning_max_clusters, uint64_t learning_max_samples,
    uint32_t anomaly_history_window,
    std::vector<amber_models::config_features> features) {
  amber_models::configure_sensor_request request{
      feature_count,           streaming_window_size,     samples_to_buffer,
      learning_rate_numerator, learning_rate_denominator, learning_max_clusters,
      learning_max_samples,    anomaly_history_window,    features};

  json j = request;
  std::string body = j.dump();
  json json_response;
  std::string slug = "/config";
  std::string url = this->license.server + slug;
  try {
    if (this->post_request(url, sensor_id, body, true, json_response) != 200) {
      return false;
    }
    response = json_response.get<amber_models::configure_sensor_response>();
  } catch (const std::exception &e) {
    fprintf(stderr, "%s\n", e.what());
    return false;
  }
  return true;
}

bool amber_sdk::get_sensor(amber_models::get_sensor_response &response,
                           const std::string &sensor_id) {
  json json_response;
  std::string slug = "/sensor";
  std::string url = this->license.server + slug;
  try {
    std::string query_params;
    if (this->get_request(url, query_params, sensor_id, json_response) != 200) {
      return false;
    }
    response = json_response.get<amber_models::get_sensor_response>();
  } catch (const std::exception &e) {
    fprintf(stderr, "%s\n", e.what());
    return false;
  }
  return true;
}

bool amber_sdk::list_sensors(amber_models::list_sensors_response &response) {
  json json_response;
  std::string no_sensor;
  std::string slug = "/sensors";
  std::string url = this->license.server + slug;
  try {
    std::string query_params;
    if (this->get_request(url, query_params, no_sensor, json_response) != 200) {
      return false;
    }
    response = json_response.get<amber_models::list_sensors_response>();
  } catch (const std::exception &e) {
    fprintf(stderr, "%s\n", e.what());
    return false;
  }
  return true;
}

bool amber_sdk::update_sensor(amber_models::update_sensor_response &response,
                              const std::string sensor_id, std::string &label) {
  amber_models::update_sensor_request request{label};
  json j = request;
  std::string body = j.dump();
  json json_response;
  std::string slug = "/sensor";
  std::string url = this->license.server + slug;
  try {
    if (this->put_request(url, sensor_id, body, json_response) != 200) {
      return false;
    }
    response = json_response.get<amber_models::update_sensor_response>();
  } catch (const std::exception &e) {
    fprintf(stderr, "%s\n", e.what());
    return false;
  }
  return true;
}

bool amber_sdk::delete_sensor(const std::string &sensor_id) {
  json json_response;
  std::string slug = "/sensor";
  std::string url = this->license.server + slug;
  try {
    if (this->delete_request(url, sensor_id, json_response) != 200) {
      return false;
    }
  } catch (const std::exception &e) {
    fprintf(stderr, "%s\n", e.what());
    return false;
  }
  return true;
}

bool amber_sdk::stream_sensor(amber_models::stream_sensor_response &response,
                              const std::string &sensor_id,
                              std::string &csvdata, bool save_image) {
  amber_models::stream_sensor_request request{save_image, csvdata};
  json j = request;
  std::string body = j.dump();
  json json_response;
  std::string slug = "/stream";
  std::string url = this->license.server + slug;
  try {
    if (this->post_request(url, sensor_id, body, true, json_response) != 200) {
      return false;
    }
    response = json_response.get<amber_models::stream_sensor_response>();
  } catch (const std::exception &e) {
    fprintf(stderr, "%s\n", e.what());
    return false;
  }
  return true;
}

bool amber_sdk::enable_learning(
    amber_models::enable_learning_response &response,
    const std::string &sensor_id, uint32_t anomaly_history_window,
    uint64_t learning_rate_numerator, uint32_t learning_rate_denominator,
    uint16_t learning_max_clusters, uint64_t learning_max_samples) {
  amber_models::enable_learning_request request{
      learning_rate_numerator, learning_rate_denominator, learning_max_clusters,
      learning_max_samples, anomaly_history_window};

  json j = request;
  std::string body = j.dump();
  json json_response;
  std::string slug = "/config";
  std::string url = this->license.server + slug;
  int code;
  try {
    if (this->put_request(url, sensor_id, body, json_response) != 200) {
      return false;
    }
    response = json_response.get<amber_models::enable_learning_response>();
  } catch (const std::exception &e) {
    fprintf(stderr, "%s\n", e.what());
    return false;
  }
  return true;
}

bool amber_sdk::pretrain_sensor(
    amber_models::pretrain_sensor_response &response,
    const std::string &sensor_id, std::string &csvdata, bool autotuneConfig,
    bool block) {
  amber_models::pretrain_sensor_request request{csvdata, autotuneConfig};
  json j = request;
  std::string body = j.dump();
  json json_response;
  std::string slug = "/pretrain";
  std::string url = this->license.server + slug;
  try {
    int response_code =
        this->post_request(url, sensor_id, body, true, json_response);
    if (response_code != 200 && response_code != 202) {
      return false;
    }
    if (!block || response_code == 200) {
      response = json_response.get<amber_models::pretrain_sensor_response>();
      return true;
    }

    amber_models::get_pretrain_response get_response;
    while (true) {
      this->get_pretrain(get_response, sensor_id);
      // get_response =
      // json_response.get<amber_models::get_pretrain_response>();
      if (get_response.state.compare("Pretraining") == 0) {
        sleep(5);
        continue;
      } else {
        response.state = get_response.state;
        response.message = get_response.message;
        return true;
      }
    }
  } catch (const std::exception &e) {
    fprintf(stderr, "%s\n", e.what());
    return false;
  }
  return true;
}

bool amber_sdk::get_pretrain(amber_models::get_pretrain_response &response,
                             const std::string &sensor_id) {
  json json_response;
  std::string slug = "/pretrain";
  std::string url = this->license.server + slug;
  try {
    std::string query_params;
    int response_code =
        this->get_request(url, query_params, sensor_id, json_response);
    if (response_code != 200 && response_code != 202) {
      return false;
    }
    response = json_response.get<amber_models::get_pretrain_response>();
  } catch (const std::exception &e) {
    fprintf(stderr, "%s\n", e.what());
    return false;
  }
  return true;
}

bool amber_sdk::get_config(amber_models::get_config_response &response,
                           const std::string &sensor_id) {
  json json_response;
  std::string slug = "/config";
  std::string url = this->license.server + slug;
  try {
    std::string query_params;
    if (this->get_request(url, query_params, sensor_id, json_response) != 200) {
      return false;
    }
    response = json_response.get<amber_models::get_config_response>();
  } catch (const std::exception &e) {
    fprintf(stderr, "%s\n", e.what());
    return false;
  }
  return true;
}

bool amber_sdk::get_version(amber_models::get_version_response &response) {
  json json_response;
  std::string no_sensor;
  std::string slug = "/version";
  std::string url = this->license.server + slug;
  try {
    std::string query_params;
    if (this->get_request(url, query_params, no_sensor, json_response) != 200) {
      return false;
    }
    response = json_response.get<amber_models::get_version_response>();
  } catch (const std::exception &e) {
    fprintf(stderr, "%s\n", e.what());
    return false;
  }
  return true;
}

bool amber_sdk::get_status(amber_models::get_status_response &response,
                           const std::string &sensor_id) {
  json json_response;
  std::string slug = "/status";
  std::string url = this->license.server + slug;
  try {
    std::string query_params;
    if (this->get_request(url, query_params, sensor_id, json_response) != 200) {
      return false;
    }
    response = json_response.get<amber_models::get_status_response>();
  } catch (const std::exception &e) {
    fprintf(stderr, "%s\n", e.what());
    return false;
  }
  return true;
}

bool amber_sdk::get_root_cause_by_idlist(
    amber_models::get_root_cause_response &response,
    const std::string &sensor_id, std::string &idlist) {
  json json_response;
  std::string slug = "/rootCause";
  std::string url = this->license.server + slug;

  if (idlist.find(std::string("[")) == std::string::npos ||
      idlist.find(std::string("]")) == std::string::npos) {
    throw amber_except("idlist should be in the form [1,2,3]");
  }
  try {
    std::string query_params = "?clusterID=" + idlist;
    if (this->get_request(url, query_params, sensor_id, json_response) != 200) {
      return false;
    }
    response = json_response.get<amber_models::get_root_cause_response>();
  } catch (const std::exception &e) {
    fprintf(stderr, "%s\n", e.what());
    return false;
  }
  return true;
}

bool amber_sdk::get_root_cause_by_patternlist(
    amber_models::get_root_cause_response &response,
    const std::string &sensor_id, std::string &patternlist) {
  json json_response;
  std::string slug = "/rootCause";
  std::string url = this->license.server + slug;
  if (patternlist.find(std::string("[[")) == std::string::npos ||
      patternlist.find(std::string("]]")) == std::string::npos) {
    throw amber_except("patternlist should be in the form [[1,2,3],[1,2,3]]");
  }
  try {
    std::string query_params = "?pattern=" + patternlist;
    if (this->get_request(url, query_params, sensor_id, json_response) != 200) {
      return false;
    }
    response = json_response.get<amber_models::get_root_cause_response>();
  } catch (const std::exception &e) {
    fprintf(stderr, "%s\n", e.what());
    return false;
  }
  return true;
}

void amber_sdk::common_curl_opts(CURL *curl, std::string &url,
                                 struct curl_slist *hs, std::string *rbufptr) {
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hs);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, rbufptr);
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, this->last_error);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, this->ssl.verify ? 1 : 0);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, this->ssl.verify ? 1 : 0);
  if (this->ssl.verify) {
    if (this->ssl.cert.empty() == false) {
      curl_easy_setopt(curl, CURLOPT_SSLCERT, this->ssl.cert.c_str());
    }
    if (this->ssl.cainfo.empty() == false) {
      curl_easy_setopt(curl, CURLOPT_CAINFO, this->ssl.cainfo.c_str());
    }
  }
}

int amber_sdk::get_request(std::string &url, std::string &query_params,
                           const std::string &sensor_id, json &response) {

  reset_last_message();
  if (!this->authenticate(response)) {
    return this->last_code;
  }
  CURL *curl;
  curl = curl_easy_init();

  std::string read_buffer;
  url += query_params;

  struct curl_slist *hs = nullptr;
  hs = curl_slist_append(hs, "Content-Type: application/json");
  hs = curl_slist_append(hs, this->auth_bear_header.c_str());
  hs = curl_slist_append(hs, user_agent);
  if (!sensor_id.empty()) {
    auto sensor_header = std::string("sensorId:" + sensor_id);
    hs = curl_slist_append(hs, sensor_header.c_str());
  }
  common_curl_opts(curl, url, hs, &read_buffer);
  curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

  // send auth request and process result
  this->last_code = curl_easy_perform(curl);
  curl_slist_free_all(hs);
  if (this->last_code == CURLE_OK) {
    // libcurl is successful, process http code
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &this->last_code);
    if (this->last_code >= 200 && this->last_code < 300) {
      // success
      response = json::parse(read_buffer);
    } else {
      // obtain error message
      json message_response = json::parse(read_buffer);
      std::string message = message_response.at("message");
      std::strncpy(this->last_error, message.c_str(),
                   sizeof(this->last_error) - 1);
    }
  }
  curl_easy_cleanup(curl);
  return this->last_code;
}

int amber_sdk::post_request(std::string &url, const std::string &sensor_id,
                            std::string &body, bool do_auth, json &response) {

  reset_last_message();
  if (do_auth) {
    if (!this->authenticate(response)) {
      return this->last_code;
    }
  }
  CURL *curl;
  curl = curl_easy_init();

  // set up request
  std::string read_buffer;
  struct curl_slist *hs = nullptr;
  hs = curl_slist_append(hs, "Content-Type: application/json");
  if (do_auth) {
    hs = curl_slist_append(hs, this->auth_bear_header.c_str());
  }
  hs = curl_slist_append(hs, user_agent);
  if (!sensor_id.empty()) {
    auto sensor_header = std::string("sensorId:" + sensor_id);
    hs = curl_slist_append(hs, sensor_header.c_str());
  }
  // compress payload if greater than 10000 bytes in length
  if (body.length() > 10000) {
    body = compress_string(body);
    hs = curl_slist_append(hs, "Content-Encoding: gzip");
  }
  common_curl_opts(curl, url, hs, &read_buffer);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.length());

  // send auth request and process result
  this->last_code = curl_easy_perform(curl);
  curl_slist_free_all(hs);
  if (this->last_code == CURLE_OK) {
    // libcurl is successful, process http code
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &this->last_code);
    if (this->last_code >= 200 && this->last_code < 300) {
      // success
      response = json::parse(read_buffer);
    } else {
      // obtain error message
      json message_response = json::parse(read_buffer);
      std::string message = message_response.at("message");
      std::strncpy(this->last_error, message.c_str(),
                   sizeof(this->last_error) - 1);
    }
  }
  curl_easy_cleanup(curl);
  return this->last_code;
}

int amber_sdk::put_request(std::string &url, const std::string &sensor_id,
                           std::string &body, json &response) {

  reset_last_message();
  if (!this->authenticate(response)) {
    return this->last_code;
  }

  CURL *curl;
  curl = curl_easy_init();

  // set up request
  std::string read_buffer;
  struct curl_slist *hs = nullptr;
  hs = curl_slist_append(hs, "Content-Type: application/json");
  hs = curl_slist_append(hs, this->auth_bear_header.c_str());
  hs = curl_slist_append(hs, user_agent);
  if (!sensor_id.empty()) {
    auto sensor_header = std::string("sensorId:" + sensor_id);
    hs = curl_slist_append(hs, sensor_header.c_str());
  }
  common_curl_opts(curl, url, hs, &read_buffer);
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());

  // send auth request and process result
  this->last_code = curl_easy_perform(curl);
  curl_slist_free_all(hs);
  if (this->last_code == CURLE_OK) {
    // libcurl is successful, process http code
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &this->last_code);
    if (this->last_code >= 200 && this->last_code < 300) {
      // success
      response = json::parse(read_buffer);
    } else {
      // obtain error message
      json message_response = json::parse(read_buffer);
      std::string message = message_response.at("message");
      std::strncpy(this->last_error, message.c_str(),
                   sizeof(this->last_error) - 1);
    }
  }
  curl_easy_cleanup(curl);
  return this->last_code;
}

int amber_sdk::delete_request(std::string &url, const std::string &sensor_id,
                              json &response) {

  reset_last_message();
  if (!this->authenticate(response)) {
    return this->last_code;
  }

  CURL *curl;
  curl = curl_easy_init();

  // set up request
  std::string read_buffer;
  struct curl_slist *hs = nullptr;
  hs = curl_slist_append(hs, "Content-Type: application/json");
  hs = curl_slist_append(hs, this->auth_bear_header.c_str());
  hs = curl_slist_append(hs, user_agent);
  auto sensor_header = std::string("sensorId:" + sensor_id);
  hs = curl_slist_append(hs, sensor_header.c_str());
  common_curl_opts(curl, url, hs, &read_buffer);
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

  // send auth request and process result
  this->last_code = curl_easy_perform(curl);
  if (this->last_code == CURLE_OK) {
    // libcurl is successful, process http code
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &this->last_code);
    if (this->last_code >= 200 && this->last_code < 300) {
      // success
      response = json::parse(read_buffer);
    } else {
      // obtain error message
      json message_response = json::parse(read_buffer);
      std::string message = message_response.at("message");
      std::strncpy(this->last_error, message.c_str(),
                   sizeof(this->last_error) - 1);
    }
  }
  curl_easy_cleanup(curl);
  return this->last_code;
}

/**
 * Authenticate client for the next hour using the credentials given at
  initialization. This acquires and stores an oauth2 token which remains
  valid for one hour and is used to authenticate all other API requests.
 * @return
 */
bool amber_sdk::authenticate(json &response) {

  if (auth_ok &&
      std::time(nullptr) + this->expires_in - 100 < this->auth_time) {
    // auth token is still good
    return true;
  }
  this->auth_ok = false;

  // create request body
  amber_models::auth_request request{this->license.username,
                                     this->license.password};
  json j = request;
  std::string body = j.dump();

  // post request
  std::string no_sensor;
  std::string slug = "/oauth2";
  std::string url = this->license.oauth_server + slug;
  try {
    this->last_code = this->post_request(url, no_sensor, body, false, response);
    if (this->last_code != 200) {
      // client-side issue
      return false;
    }
    this->auth_ok = true;

    // process response
    this->auth = response.get<amber_models::auth_response>();
    this->expires_in = std::stoul(this->auth.expiresIn, nullptr, 0);
    this->auth_time = std::time(nullptr) + this->expires_in;
    this->auth_bear_header =
        std::string("Authorization: Bearer " + this->auth.idToken);
  } catch (const std::exception &e) {
    fprintf(stderr, "%s\n", e.what());
    return false;
  }
  return true;
}

std::string compress_string(const std::string &str) {
  int compressionlevel = Z_BEST_COMPRESSION;
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
