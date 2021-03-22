#include "amber_sdk.h"
#include <wordexp.h>
#include <fstream>
#include <sstream>

using json = nlohmann::json;

const char* user_agent = "User-Agent: amber-cpp-sdk";

/**
 * Main client which interfaces with the Amber cloud. Amber account
 * credentials are discovered within a .Amber.license file located in the
 * home directory, or optionally overridden using environment variables.
 *
 * @param license_id: license identifier label found within .Amber.license file
 * @param license_file: path to .Amber.license file
 * @param verify_cert: whether to verify the ssl certificate or not
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
 *   AMBER_SSL_VERIFY: Either a boolean, in which case it controls whether we verify the server’s TLS certificate, or a string, in which case it must be a path to a CA bundle to use
 *
 */
amber_sdk::amber_sdk(const char *license_id, const char *license_file, bool verify_cert, const char *cert, const char *capath) {
    this->auth_time = 0;
    this->last_code = 0;
    this->last_error[0] = '\0';
    this->expires_in = 0;
    this->auth_ok = false;

    char *env_license_file = getenv("AMBER_LICENSE_FILE");
    char *env_license_id = getenv("AMBER_LICENSE_ID");
    char *env_username = getenv("AMBER_USERNAME");
    char *env_password = getenv("AMBER_PASSWORD");
    char *env_server = getenv("AMBER_SERVER");

    char *env_cert = getenv("AMBER_SSL_CERT");
    char *env_verify = getenv("AMBER_SSL_VERIFY");

    // certificates
    this->set_cert(env_cert ? env_cert : cert);

    // verification initialize
    this->verify_certificate(verify_cert);
    this->set_capath(capath);
    // verification override via env variable?
    if (env_verify) {
        this->verify_certificate(true);
        if (strcasecmp("false", env_verify) == 0) {
            this->verify_certificate(false);
            this->set_capath("");
        } else if (strcasecmp("true", env_verify) != 0) {
            this->set_capath(env_verify);
        }
    }

    // if username, password and server are all specified via environment, we're done here
    if (env_username && env_password && env_server) {
        this->license.username = env_username;
        this->license.password = env_password;
        this->license.server = env_server;
        return;
    }

    // otherwise we acquire either or both of them from license file
    license_file = env_license_file ? env_license_file : license_file;
    license_id = env_license_id ? env_license_id : license_id;

    // expand license file to full path
    wordexp_t exp_result;
    wordexp(license_file, &exp_result, 0);
    license_file = exp_result.we_wordv[0];

    // open up license file
    std::ifstream file(license_file);
    if (!file.is_open()) {
        throw amber_except("unable to open %s", license_file);
    }

    // load license file as json
    json license_json;
    file >> license_json;

    // locate license entry in license file
    json::iterator it = license_json.find(license_id);
    if (it == license_json.end()) {
        throw amber_except("license_id '%s' not found in '%s'", license_id, license_file);
    }

    // rewrite as license_entry structure
    json amber_entry = *it;
    this->license = amber_entry.get<amber_models::license_entry>();
}

amber_sdk::~amber_sdk() = default;

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string *) userp)->append((char *) contents, size * nmemb);
    return size * nmemb;
}

bool amber_sdk::create_sensor(amber_models::create_sensor_response &response, std::string &label) {
    amber_models::create_sensor_request request(label);
    json j = request;
    std::string body = j.dump();
    json json_response;
    std::string no_sensor;
    std::string slug = "/sensor";
    if (this->post_request(slug, no_sensor, body, true, json_response) != 200) {
        return false;
    }
    response = json_response.get<amber_models::create_sensor_response>();
    return true;
}

bool amber_sdk::configure_sensor(amber_models::configure_sensor_response &response, std::string &sensor_id,
                                 uint16_t feature_count, uint16_t streaming_window_size,
                                 uint32_t samples_to_buffer, uint64_t learning_rate_numerator,
                                 uint32_t learning_rate_denominator, uint16_t learning_max_clusters,
                                 uint64_t learning_max_samples) {
    amber_models::configure_sensor_request request{feature_count, streaming_window_size, samples_to_buffer,
                                                   learning_rate_numerator, learning_rate_denominator,
                                                   learning_max_clusters, learning_max_samples};
    json j = request;
    std::string body = j.dump();
    json json_response;
    std::string slug = "/config";
    if (this->post_request(slug, sensor_id, body, true, json_response) != 200) {
        return false;
    }
    response = json_response.get<amber_models::configure_sensor_response>();
    return true;
}

bool amber_sdk::get_sensor(amber_models::get_sensor_response &response, std::string &sensor_id) {
    json json_response;
    std::string slug = "/sensor";
    if (this->get_request(slug, sensor_id, json_response) != 200) {
        return false;
    }
    response = json_response.get<amber_models::get_sensor_response>();
    return true;
}

bool amber_sdk::list_sensors(amber_models::list_sensors_response &response) {
    json json_response;
    std::string no_sensor;
    std::string slug = "/sensors";
    if (this->get_request(slug, no_sensor, json_response) != 200) {
        return false;
    }
    response = json_response.get<amber_models::list_sensors_response>();
    return true;
}

bool
amber_sdk::update_sensor(amber_models::update_sensor_response &response, std::string sensor_id, std::string &label) {
    amber_models::update_sensor_request request{label};
    json j = request;
    std::string body = j.dump();
    json json_response;
    std::string slug = "/sensor";
    if (this->put_request(slug, sensor_id, body, json_response) != 200) {
        return false;
    }
    response = json_response.get<amber_models::update_sensor_response>();
    return true;
}

bool amber_sdk::delete_sensor(std::string &sensor_id) {
    json json_response;
    std::string slug = "/sensor";
    return this->delete_request(slug, sensor_id, json_response) == 200;
}

bool
amber_sdk::stream_sensor(amber_models::stream_sensor_response &response, std::string &sensor_id, std::string &csvdata) {
    amber_models::stream_sensor_request request{csvdata};
    json j = request;
    std::string body = j.dump();
    json json_response;
    std::string slug = "/stream";
    if (this->post_request(slug, sensor_id, body, true, json_response) != 200) {
        return false;
    }
    response = json_response.get<amber_models::stream_sensor_response>();
    return true;
}

bool amber_sdk::get_config(amber_models::get_config_response &response, std::string &sensor_id) {
    json json_response;
    std::string slug = "/config";
    if (this->get_request(slug, sensor_id, json_response) != 200) {
        return false;
    }
    response = json_response.get<amber_models::get_config_response>();
    return true;
}

bool amber_sdk::get_status(amber_models::get_status_response &response, std::string &sensor_id) {
    json json_response;
    std::string slug = "/status";
    if (this->get_request(slug, sensor_id, json_response) != 200) {
        return false;
    }
    response = json_response.get<amber_models::get_status_response>();
    return true;
}

void amber_sdk::common_curl_opts(CURL *curl, std::string &url, struct curl_slist *hs, std::string *rbufptr) {
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
        if (this->ssl.capath.empty() == false) {
            curl_easy_setopt(curl, CURLOPT_CAINFO, this->ssl.capath.c_str());
        }
    }
}

int amber_sdk::get_request(std::string &slug, std::string &sensor_id, json &response) {

    reset_last_message();
    if (!this->authenticate(response)) {
        return this->last_code;
    }
    CURL *curl;
    curl = curl_easy_init();

    std::string read_buffer;
    std::string url = this->license.server + slug;
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
            std::strncpy(this->last_error, message.c_str(), sizeof(this->last_error) - 1);
        }
    }
    curl_easy_cleanup(curl);
    return this->last_code;
}

int
amber_sdk::post_request(std::string &slug, std::string &sensor_id, std::string &body, bool do_auth, json &response) {

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
    std::string url = this->license.server + slug;
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
            std::strncpy(this->last_error, message.c_str(), sizeof(this->last_error) - 1);
        }
    }
    curl_easy_cleanup(curl);
    return this->last_code;
}

int amber_sdk::put_request(std::string &slug, std::string &sensor_id, std::string &body, json &response) {

    reset_last_message();
    if (!this->authenticate(response)) {
        return this->last_code;
    }

    CURL *curl;
    curl = curl_easy_init();

    // set up request
    std::string read_buffer;
    std::string url = this->license.server + slug;
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
            std::strncpy(this->last_error, message.c_str(), sizeof(this->last_error) - 1);
        }
    }
    curl_easy_cleanup(curl);
    return this->last_code;
}

int amber_sdk::delete_request(std::string &slug, std::string &sensor_id, json &response) {

    reset_last_message();
    if (!this->authenticate(response)) {
        return this->last_code;
    }

    CURL *curl;
    curl = curl_easy_init();

    // set up request
    std::string read_buffer;
    std::string url = this->license.server + slug;
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
            std::strncpy(this->last_error, message.c_str(), sizeof(this->last_error) - 1);
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

    if (auth_ok && std::time(nullptr) + this->expires_in - 100 < this->auth_time) {
        // auth token is still good
        return true;
    }
    this->auth_ok = false;

    // create request body
    amber_models::auth_request request{this->license.username, this->license.password};
    json j = request;
    std::string body = j.dump();

    // post request
    std::string no_sensor;
    std::string slug = "/oauth2";
    this->last_code = this->post_request(slug, no_sensor, body, false, response);
    if (this->last_code != 200) {
        // client-side issue
        return false;
    }
    this->auth_ok = true;

    // process response
    this->auth = response.get<amber_models::auth_response>();
    this->expires_in = std::stoul(this->auth.expiresIn, nullptr, 0);
    this->auth_time = std::time(nullptr) + this->expires_in;
    this->auth_bear_header = std::string("Authorization: Bearer " + this->auth.idToken);

    return true;
}
