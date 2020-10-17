#include "models.h"
#include <iostream>

namespace amber_models {

    void to_json(json &j, const delete_sensor_response &r) {
    }
    void from_json(const json &j, delete_sensor_response &r) {
        memset(&r, 0, sizeof(r));
    }

    void to_json(json &j, const stream_sensor_response &r) {
    }

    void from_json(const json &j, stream_sensor_response &r) {
        memset(&r, 0, sizeof(r));
    }

    void to_json(json &j, const get_status_response &r) {
    }

    void from_json(const json &j, get_status_response &r) {
        memset(&r, 0, sizeof(r));
    }
}
