#include "sdk.h"
#include <chrono>
#include <ctime>
#include <wordexp.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include "nlohmann/json.hpp"

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
    this->token = "";
    // this->reauth_time

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

    std::ifstream file(license_file);
    if (!file.is_open()) {
        throw amber_except("unable to open %s", license_file);
    }

    json license_json;
    file >> license_json;

    // locate license entry in license file
    json::iterator it = license_json.find(license_id);
    if (it->empty()) {
        throw amber_except("license_id '%s' not found in '%s'", license_id, license_file);
    }
    this->license.from_json(*it);
}

amber_sdk::~amber_sdk() {
}

CURL *amber_sdk::create_sensor(std::string label) {
    CURLcode res;
    return NULL;
}

CURL *amber_sdk::update_label(std::string sensor_id, std::string label) {
    CURLcode res;
    return NULL;
}

CURL *amber_sdk::delete_sensor(std::string sensor_id) {
    CURLcode res;
    return NULL;
}

CURL *amber_sdk::list_sensors() {
    CURLcode res;
    this->authenticate();
    return NULL;
}

CURL *amber_sdk::configure_sensor(std::string sensor_id, int feature_count, int streaming_window_size,
                                  int samples_to_buffer, int learning_rate_numerator,
                                  int learning_rate_denominator, int learning_max_clusters,
                                  int learning_max_samples) {
    CURLcode res;
    return NULL;
}

CURL *amber_sdk::stream_sensor(std::string sensor_id, std::string csvdata) {
    CURLcode res;
    return NULL;
}

CURL *amber_sdk::get_sensor(std::string sensor_id) {
    CURLcode res;
    return NULL;
}

CURL *amber_sdk::get_config(std::string sensor_id) {
    CURLcode res;
    return NULL;
}

CURL *amber_sdk::get_status(std::string sensor_id) {
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

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    // set up request
    std::string url = this->license.server + "/oauth2";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    struct curl_slist *hs = NULL;
    hs = curl_slist_append(hs, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hs);
    json license_json;
    this->license.to_json(license_json);
    std::cout << license_json.dump().c_str() << "\n";
    auth_request request;
    request.data.username = this->license.username;
    request.data.password = this->license.password;
    json payload;
    request.to_json(payload);
    std::cout << payload.dump().c_str() << "\n";
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.dump().c_str());

    res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);

    return true;
}

