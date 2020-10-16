#ifndef AMBER_CPP_SDK_MODELS_H
#define AMBER_CPP_SDK_MODELS_H

#include <string>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace amber_models {

    // amber license entry
    struct license_entry {
        std::string username;
        std::string password;
        std::string server;
    };

    void to_json(json &j, const license_entry &r);

    void from_json(const json &j, license_entry &r);

    // authentication request
    struct auth_request {
        std::string username;
        std::string password;
    };

    void to_json(json &j, const auth_request &r);

    void from_json(const json &j, auth_request &r);

    // authentication request
    // authentication response
    struct auth_response {
        std::string idToken;
        std::string expiresIn;
        std::string refreshToken;
    };

    void to_json(json &j, const auth_response &r);

    void from_json(const json &j, auth_response &r);

    // sensor instance
    struct sensor_instance {
        std::string label_id;
        std::string sensor_id;
        std::string tenant_id;
    };

    void to_json(json &j, const sensor_instance &r);

    void from_json(const json &j, sensor_instance &r);

    // sensor list
    typedef std::vector<sensor_instance> sensor_list;

    // post sensor request
    struct create_sensor_request {
        std::string label_id;
    };

    void to_json(json &j, const create_sensor_request &r);

    void from_json(const json &j, create_sensor_request &r);

    // post sensor response
    struct create_sensor_response {
        std::string sensor_id;
        std::string tenant_id;
        std::string label_id;
    };

    void to_json(json &j, const create_sensor_response &r);

    void from_json(const json &j, create_sensor_response &r);

    // get sensor response
    struct usage_element {
        uint64_t calls_total;
        uint64_t calls_this_period;
        std::string last_called;
    };

    struct post_stream_element : usage_element {
        uint64_t samples_total;
        uint64_t samples_this_period;
    };

    struct get_sensor_response {
        std::string sensor_id;
        std::string tenant_id;
        std::string label_id;
        struct usage_info {
            usage_element put_sensor;
            usage_element post_config;
            usage_element get_sensor;
            usage_element get_config;
            usage_element get_status;
            post_stream_element post_stream;
        } usage_info;
    };

    void to_json(json &j, const get_sensor_response &r);

    void from_json(const json &j, get_sensor_response &r);

    // update_sensor_request
    struct update_sensor_request {
        std::string label_id;
    };

    void to_json(json &j, const update_sensor_request &r);

    void from_json(const json &j, update_sensor_request &r);

    // update_sensor_response
    struct update_sensor_response {
        std::string label_id;
        std::string sensor_id;
        std::string tenant_id;
    };

    void to_json(json &j, const update_sensor_response &r);

    void from_json(const json &j, update_sensor_response &r);
}

#endif //AMBER_CPP_SDK_MODELS_H
