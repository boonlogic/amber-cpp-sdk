#include "sdk.h"
#include <chrono>
#include <ctime>
#include <wordexp.h>
#include <fstream>
#include <sstream>
#include "nlohmann/json.hpp"
#include "models.h"

using json = nlohmann::json;

/**
 * Main client which interfaces with the Amber cloud. Amber account
 * credentials are discovered within a .Amber.license file located in the
 * home directory, or optionally overridden using environment variables.
 *
 * @param license_id: license identifier label found within .Amber.license file
 * @param license_file: path to .Amber.license file
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
 */
amber_sdk::amber_sdk(const char *license_id, const char *license_file) {
    this->curl = curl_easy_init();
    this->reauth_time = 0;

    char *env_license_file = getenv("AMBER_LICENSE_FILE");
    char *env_license_id = getenv("AMBER_LICENSE_ID");
    char *env_username = getenv("AMBER_USERNAME");
    char *env_password = getenv("AMBER_PASSWORD");
    char *env_server = getenv("AMBER_SERVER");

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
    if (it->empty()) {
        throw amber_except("license_id '%s' not found in '%s'", license_id, license_file);
    }

    // rewrite as license_entry structure
    json amber_entry = *it;
    this->license = amber_entry.get<amber_models::license_entry>();
}

amber_sdk::~amber_sdk() {
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string *) userp)->append((char *) contents, size * nmemb);
    return size * nmemb;
}

amber_models::create_sensor_response *amber_sdk::create_sensor(std::string label) {
    amber_models::create_sensor_request request{label};
    json j = request;
    std::string body = j.dump();
    auto post_response = this->post_request(std::string("/sensor"), std::string(""), body);
    auto response = post_response.get<amber_models::create_sensor_response>();
    return &response;
}

amber_models::get_sensor_response *amber_sdk::get_sensor(std::string sensor_id) {
    auto get_response = this->get_request(std::string("/sensor"), sensor_id);
    auto response = get_response.get<amber_models::get_sensor_response>();
    return &response;
}

amber_models::sensor_list *amber_sdk::list_sensors() {
    auto response = this->get_request(std::string("/sensors"), "");
    auto list_response = new amber_models::sensor_list;
    for (json::iterator it = response.begin(); it != response.end(); ++it) {
        list_response->push_back(it->get<amber_models::sensor_instance>());
    }
    return list_response;
}

amber_models::update_sensor_response* amber_sdk::update_sensor(std::string sensor_id, std::string label) {
    amber_models::update_sensor_request request{label};
    json j = request;
    std::string body = j.dump();
    auto put_response = this->put_request(std::string("/sensor"), sensor_id, body);
    auto response = put_response.get<amber_models::update_sensor_response>();
    return &response;
}

CURL *amber_sdk::delete_sensor(std::string sensor_id) {
    this->authenticate();
    CURLcode res;
    return NULL;
}

json amber_sdk::get_request(std::string slug, std::string sensor_id) {
    this->authenticate();
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    std::string read_buffer;
    std::string url = this->license.server + slug;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    struct curl_slist *hs = NULL;
    hs = curl_slist_append(hs, "Content-Type: application/json");
    hs = curl_slist_append(hs, this->auth_bear_header.c_str());
    if (sensor_id != "") {
        auto sensor_header = std::string("sensorId:" + sensor_id);
        hs = curl_slist_append(hs, sensor_header.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hs);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);

    // send request
    res = curl_easy_perform(curl);
    curl_slist_free_all(hs);
    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        fprintf(stderr, "%s\n\n", curl_easy_strerror(res));
        return NULL;
    }

    curl_easy_cleanup(curl);

    json response = json::parse(read_buffer);
    return response;
}

json amber_sdk::post_request(std::string slug, std::string sensor_id, std::string body, bool do_auth) {
    if (do_auth) {
        this->authenticate();
    }
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    // set up request
    std::string read_buffer;
    std::string url = this->license.server + slug;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    struct curl_slist *hs = NULL;
    hs = curl_slist_append(hs, "Content-Type: application/json");
    if (do_auth) {
        hs = curl_slist_append(hs, this->auth_bear_header.c_str());
    }
    if (sensor_id != "") {
        auto sensor_header = std::string("sensorId:" + sensor_id);
        hs = curl_slist_append(hs, sensor_header.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hs);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.length());

    // send auth request
    res = curl_easy_perform(curl);
    curl_slist_free_all(hs);
    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        fprintf(stderr, "%s\n\n", curl_easy_strerror(res));
        return NULL;
    }

    curl_easy_cleanup(curl);
    json response = json::parse(read_buffer);

    return response;
}

json amber_sdk::put_request(std::string slug, std::string sensor_id, std::string body) {
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    // set up request
    std::string read_buffer;
    std::string url = this->license.server + slug;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    struct curl_slist *hs = NULL;
    hs = curl_slist_append(hs, "Content-Type: application/json");
    hs = curl_slist_append(hs, this->auth_bear_header.c_str());
    if (sensor_id != "") {
        auto sensor_header = std::string("sensorId:" + sensor_id);
        hs = curl_slist_append(hs, sensor_header.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hs);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);

    res = curl_easy_perform(curl);
    curl_slist_free_all(hs);
    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        fprintf(stderr, "%s\n\n", curl_easy_strerror(res));
        return NULL;
    }

    curl_easy_cleanup(curl);
    json response = json::parse(read_buffer);

    return response;
}

CURL *amber_sdk::configure_sensor(std::string sensor_id, int feature_count, int streaming_window_size,
                                  int samples_to_buffer, int learning_rate_numerator,
                                  int learning_rate_denominator, int learning_max_clusters,
                                  int learning_max_samples) {
    this->authenticate();
    CURLcode res;
    return NULL;
}

CURL *amber_sdk::stream_sensor(std::string sensor_id, std::string csvdata) {
    this->authenticate();
    CURLcode res;
    return NULL;
}

CURL *amber_sdk::get_config(std::string sensor_id) {
    this->authenticate();
    CURLcode res;
    return NULL;
}

CURL *amber_sdk::get_status(std::string sensor_id) {
    this->authenticate();
    CURLcode res;
    return NULL;
}

/**
 * Authenticate client for the next hour using the credentials given at
  initialization. This acquires and stores an oauth2 token which remains
  valid for one hour and is used to authenticate all other API requests.
 * @return
 */
bool amber_sdk::authenticate() {

    // note: we can't use
    if (std::time(nullptr) + this->expires_in - 100 < this->reauth_time) {
        return true;
    }

    // create request body
    amber_models::auth_request request{this->license.username, this->license.password};
    json j = request;
    std::string body = j.dump();

    // post request
    auto response = this->post_request(std::string("/oauth2"), std::string(""), body, false /* do_auth=false */);

    // process response
    amber_models::auth_response resp = response.get<amber_models::auth_response>();
    this->id_token = resp.idToken;
    this->refresh_token = resp.refreshToken;
    this->expires_in = std::stoi(resp.expiresIn);
    this->reauth_time = std::time(nullptr) + this->expires_in;
    this->auth_bear_header = std::string("Authorization: Bearer " + this->id_token);

    return true;
}
