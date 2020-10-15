#pragma once

#include <curl/curl.h>
#include <string>
#include <chrono>
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

    CURL *create_sensor(std::string label);

    CURL *update_label(std::string sensor_id, std::string label);

    CURL *delete_sensor(std::string sensor_id);

    CURL *list_sensors();

    CURL *configure_sensor(std::string sensor_id, int feature_count, int streaming_window_size,
                           int samples_to_buffer, int learning_rate_numerator,
                           int learning_rate_denominator, int learning_max_clusters,
                           int learning_max_samples);

    CURL *stream_sensor(std::string sensor_id, std::string csvdata);

    CURL *get_sensor(std::string sensor_id);

    CURL *get_config(std::string sensor_id);

    CURL *get_status(std::string sensor_id);

protected:
    amber_models::license_entry license;

private:
    bool authenticate();

    std::chrono::high_resolution_clock reauth_time;

    std::string token;

    CURL *curl;
};
