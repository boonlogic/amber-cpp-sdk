#ifndef AMBER_CPP_SDK_AMBER_SDK_H
#define AMBER_CPP_SDK_AMBER_SDK_H

#include <curl/curl.h>
#include <stdarg.h>
#include <string>
#include <ctime>
#include <exception>
#include "nlohmann/json.hpp"
#include "amber_models.h"

using json = nlohmann::json;

class amber_except : public std::exception {
public:
    amber_except(const char *fmt, ...) {
        va_list args;
        va_start (args, fmt);
        vsprintf(this->buffer, fmt, args);
        va_end (args);
    };

    char *what() {
        return this->buffer;
    }

    char buffer[256];
};

class amber_sdk {
public:
    amber_sdk(const char *license_id = "default", const char *license_file = "~/.Amber.license", bool verify_cert = true);

    ~amber_sdk();

    void verify_certificate(bool verify_cert) { certificate.verify = verify_cert; }

    bool create_sensor(amber_models::create_sensor_response &response, std::string &label);

    bool get_sensor(amber_models::get_sensor_response &response, std::string &sensor_id);

    bool list_sensors(amber_models::list_sensors_response &response);

    bool update_sensor(amber_models::update_sensor_response &response, std::string sensor_id, std::string &label);

    bool delete_sensor(std::string &sensor_id);

    bool configure_sensor(amber_models::configure_sensor_response &response, std::string &sensor_id,
                          uint16_t feature_count = 1, uint16_t streaming_window_size = 25,
                          uint32_t samples_to_buffer = 1000, uint64_t learning_rate_numerator = 0,
                          uint32_t learning_rate_denominator = 2000, uint16_t learning_max_clusters = 1000,
                          uint64_t learning_max_samples = 1000000);

    bool stream_sensor(amber_models::stream_sensor_response &response, std::string &sensor_id, std::string &csvdata);

    bool get_config(amber_models::get_config_response &response, std::string &sensor_id);

    bool get_status(amber_models::get_status_response &response, std::string &sensor_id);

    int last_code;
    char last_error[CURL_ERROR_SIZE];

    void reset_last_message() {
        last_code = 0;
        last_error[0] = 0;
    }

private:
    void common_curl_opts(CURL *curl, std::string &url, struct curl_slist *hs, std::string *rbufptr);

    bool authenticate(json &response);

    int get_request(std::string &slug, std::string &sensor_id, json &response);

    int post_request(std::string &slug, std::string &sensor_id, std::string &body, bool do_auth, json &response);

    int put_request(std::string &slug, std::string &sensor_id, std::string &body, json &response);

    int delete_request(std::string &slug, std::string &sensor_id, json &response);

    amber_models::license_entry license;
    amber_models::auth_response auth;
    uint64_t expires_in;
    bool auth_ok;
    std::time_t auth_time;
    std::string auth_bear_header;

    // certificate options
    struct {
        bool verify;
    } certificate;
};

#endif // AMBER_CPP_SDK_AMBER_SDK_H
