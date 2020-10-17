#ifndef AMBER_CPP_SDK_SDK_H
#define AMBER_CPP_SDK_SDK_H

#include <curl/curl.h>
#include <string>
#include <ctime>
#include <exception>
#include "nlohmann/json.hpp"
#include "models.h"

using json = nlohmann::json;

class amber_except : public std::exception {
public:
    amber_except(const char *fmt, ...) {
        va_list args;
        va_start (args, fmt);
        vsprintf(this->buffer, fmt, args);
        perror(this->buffer);
        va_end (args);
    };

    char *what() {
        return this->buffer;
    }

    char buffer[256];
};

class amber_sdk {
public:
    amber_sdk(const char *license_id = "default", const char *license_file = "~/.Amber.license");

    ~amber_sdk();

    amber_models::create_sensor_response create_sensor(std::string label);

    amber_models::get_sensor_response *get_sensor(std::string sensor_id);

    amber_models::sensor_list *list_sensors();

    amber_models::update_sensor_response *update_sensor(std::string sensor_id, std::string label);

    amber_models::delete_sensor_response *delete_sensor(std::string sensor_id);

    amber_models::configure_sensor_response *
    configure_sensor(std::string sensor_id, uint16_t feature_count=1, uint16_t streaming_window_size=25,
                     uint32_t samples_to_buffer=1000, uint64_t learning_rate_numerator=0,
                     uint32_t learning_rate_denominator=2000, uint16_t learning_max_clusters=1000,
                     uint64_t learning_max_samples=1000000);

    amber_models::stream_sensor_response *stream_sensor(std::string sensor_id, std::string csvdata);

    amber_models::get_config_response *get_config(std::string sensor_id);

    amber_models::get_status_response *get_status(std::string sensor_id);

private:
    bool authenticate();

    std::string auth_bear_header;

    json get_request(std::string slug, std::string sensor_id);

    json post_request(std::string slug, std::string sensor_id, std::string body, bool do_auth = true);

    json put_request(std::string slug, std::string sensor_id, std::string body);

    // json delete_request(std::string slug, std::string sensor_id);

    amber_models::license_entry license;
    amber_models::auth_response auth;
    uint64_t expires_in;
    std::time_t auth_time;

    CURL *curl;
};

#endif // AMBER_CPP_SDK_SDK_H
