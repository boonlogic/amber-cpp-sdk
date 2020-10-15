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
}

#endif //AMBER_CPP_SDK_MODELS_H
