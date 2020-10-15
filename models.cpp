#include "models.h"
#include <iostream>

namespace amber_models {

    // license_entry processing
    void to_json(json &j, const license_entry &r) {
        j = json{{"username", r.username},
                 {"password", r.password},
                 {"server",   r.server}};
    }

    void from_json(const json &j, license_entry &r) {
        j.at("username").get_to(r.username);
        j.at("password").get_to(r.password);
        j.at("server").get_to(r.server);
    }

    // authentication request
    void to_json(json &j, const auth_request &r) {
        j = json{{"username", r.username},
                 {"password", r.password}
        };
    }

    void from_json(const json &j, auth_request &r) {
        j.at("username").get_to(r.username);
        j.at("password").get_to(r.password);
    }

    // authentication response
    void to_json(json &j, const auth_response &r) {
        j = json{{"idToken", r.idToken}
        };
    }

    void from_json(const json &j, auth_response &r) {
        j.at("idToken").get_to(r.idToken);
        j.at("refreshToken").get_to(r.refreshToken);
        j.at("expiresIn").get_to(r.expiresIn);
    }

    // sensor_instance
    void to_json(json &j, const sensor_instance &r) {
        j = json{{"label",    r.label_id},
                 {"sensorId", r.sensor_id},
                 {"tenantId", r.tenant_id}
        };
    }

    void from_json(const json &j, sensor_instance &r) {
        try {
            j.at("label").get_to(r.label_id);
        } catch (nlohmann::detail::out_of_range) {} // not required
        j.at("sensorId").get_to(r.sensor_id);
        j.at("tenantId").get_to(r.tenant_id);
    }

    // post sensor request
    void to_json(json &j, const create_sensor_request &r) {
        j = json{{"label", r.label_id}
        };
    }

    void from_json(const json &j, create_sensor_request &r) {
        try {
            j.at("label").get_to(r.label_id);
        } catch (nlohmann::detail::out_of_range) {} // not required
    }

    // post sensor response
    void to_json(json &j, const create_sensor_response &r) {
        j = json{{"sensorId", r.sensor_id}
        };
    }

    void from_json(const json &j, create_sensor_response &r) {
        try {
            j.at("label").get_to(r.label_id);
        } catch (nlohmann::detail::out_of_range) {} // not required
        j.at("sensorId").get_to(r.sensor_id);
        j.at("tenantId").get_to(r.tenant_id);
    }
}
