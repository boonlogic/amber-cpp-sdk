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
    }
}
