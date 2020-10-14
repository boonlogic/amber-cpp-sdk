#include <curl/curl.h>
#include <string>
#include <chrono>
#include <ctime>
#include <exception>
#include "nlohmann/json.hpp"

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

class license_entry {

public:
    // a simple struct to model a person
    std::string username;
    std::string password;
    std::string server;

    void to_json(json &j) {
        j = json{{"username", this->username},
                 {"password", this->password},
                 {"server",   this->server}};
    }

    void from_json(const json &j) {
        j.at("username").get_to(this->username);
        j.at("password").get_to(this->password);
        j.at("server").get_to(this->server);
    }
};

class auth_request {

public:
    struct auth_struct {
        std::string username;
        std::string password;
    } data;

    void to_json(json &j) {
        j = json{{"username",  this->data.username,
                  "password",  this->data.password}
        };
    }

    void from_json(const json &j) {
        j.at("username").get_to(this->data.username);
        j.at("password").get_to(this->data.password);
    }
};

class auth_response {

public:
    std::string idToken;

    void to_json(json &j) {
        j = json{{"idToken",  this->idToken}
                 };
    }

    void from_json(const json &j) {
        j.at("idToken").get_to(this->idToken);
    }
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
    license_entry license;

private:
    bool authenticate();

    std::chrono::high_resolution_clock reauth_time;

    std::string token;

    CURL *curl;
};
