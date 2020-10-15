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

    amber_models::create_sensor_response *create_sensor(std::string label);

    amber_models::get_sensor_response *get_sensor(std::string sensor_id);

    amber_models::sensor_list *list_sensors();

    CURL *update_label(std::string sensor_id, std::string label);

    CURL *delete_sensor(std::string sensor_id);


    CURL *configure_sensor(std::string sensor_id, int feature_count, int streaming_window_size,
                           int samples_to_buffer, int learning_rate_numerator,
                           int learning_rate_denominator, int learning_max_clusters,
                           int learning_max_samples);

    CURL *stream_sensor(std::string sensor_id, std::string csvdata);

    CURL *get_config(std::string sensor_id);

    CURL *get_status(std::string sensor_id);

protected:
    amber_models::license_entry license;

private:
    bool authenticate();
    std::string auth_bear_header;
    json get_request(std::string slug, std::string sensor_id="");
    json post_request(std::string slug, std::string body, bool do_auth=true);


    std::time_t reauth_time;

    std::string id_token;
    std::string refresh_token;
    uint32_t expires_in;

    CURL *curl;
};
#endif // AMBER_CPP_SDK_SDK_H
