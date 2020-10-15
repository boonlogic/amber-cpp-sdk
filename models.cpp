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
        memset(&r, 0, sizeof(r));
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
        memset(&r, 0, sizeof(r));
        j.at("username").get_to(r.username);
        j.at("password").get_to(r.password);
    }

    // authentication response
    void to_json(json &j, const auth_response &r) {
        j = json{{"idToken", r.idToken}
        };
    }

    void from_json(const json &j, auth_response &r) {
        memset(&r, 0, sizeof(r));
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
        memset(&r, 0, sizeof(r));
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
        memset(&r, 0, sizeof(r));
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
        memset(&r, 0, sizeof(r));
        try {
            j.at("label").get_to(r.label_id);
        } catch (nlohmann::detail::out_of_range) {} // not required
        j.at("sensorId").get_to(r.sensor_id);
        j.at("tenantId").get_to(r.tenant_id);
    }

    void to_json(json &j, const get_sensor_response &r) {
        j = json{{"sensorId",  r.sensor_id},
                 {"tenantId",  r.tenant_id},
                 {"label",     r.label_id},
                 {"usageInfo", {
                                       {"putSensor", {
                                                             {"callsTotal", r.usage_info.put_sensor.calls_total},
                                                             {"callsThisPeriod", r.usage_info.put_sensor.calls_total},
                                                             {"lastCalled", r.usage_info.put_sensor.last_called}
                                                     }},
                                       {"postConfig", {
                                                              {"callsTotal", r.usage_info.post_config.calls_total},
                                                              {"callsThisPeriod", r.usage_info.post_config.calls_total},
                                                              {"lastCalled", r.usage_info.post_config.last_called}
                                                      }},
                                       {"getSensor", {
                                                             {"callsTotal", r.usage_info.get_sensor.calls_total},
                                                             {"callsThisPeriod", r.usage_info.get_sensor.calls_total},
                                                             {"lastCalled", r.usage_info.get_sensor.last_called}
                                                     }},
                                       {"getConfig", {
                                                             {"callsTotal", r.usage_info.get_config.calls_total},
                                                             {"callsThisPeriod", r.usage_info.get_config.calls_total},
                                                             {"lastCalled", r.usage_info.get_config.last_called}
                                                     }},
                                       {"getStatus", {
                                                             {"callsTotal", r.usage_info.get_status.calls_total},
                                                             {"callsThisPeriod", r.usage_info.get_status.calls_total},
                                                             {"lastCalled", r.usage_info.get_status.last_called}
                                                     }},
                                       {"postStream", {
                                                              {"callsTotal", r.usage_info.post_stream.calls_total},
                                                              {"callsThisPeriod", r.usage_info.post_stream.calls_total},
                                                              {"lastCalled", r.usage_info.post_stream.last_called},
                                                              {"samplesTotal", r.usage_info.post_stream.samples_total},
                                                              {"samplesThisPeriod", r.usage_info.post_stream.samples_this_period}
                                                      }}
                               }
                 }
        };
    }

    void from_json(const json &j, get_sensor_response &r) {
        memset(&r, 0, sizeof(r));
        j.at("sensorId").get_to(r.sensor_id);
        j.at("tenantId").get_to(r.tenant_id);
        try {
            j.at("label").get_to(r.label_id);
        } catch (nlohmann::detail::out_of_range) {} // not required
        std::cout << j.dump() << "\n";
        auto usage_info = j.at("usageInfo");

        auto put_sensor = usage_info.at("putSensor");
        if (!put_sensor.empty()) {
            put_sensor.at("callsTotal").get_to(r.usage_info.put_sensor.calls_total);
            put_sensor.at("callsThisPeriod").get_to(r.usage_info.put_sensor.calls_this_period);
            put_sensor.at("lastCalled").get_to(r.usage_info.put_sensor.last_called);
        }

        auto post_config = usage_info.at("postConfig");
        if (!post_config.empty()) {
            post_config.at("callsTotal").get_to(r.usage_info.post_config.calls_total);
            post_config.at("callsThisPeriod").get_to(r.usage_info.post_config.calls_this_period);
            post_config.at("lastCalled").get_to(r.usage_info.post_config.last_called);
        }

        auto get_sensor = usage_info.at("getSensor");
        if (!get_sensor.empty()) {
            get_sensor.at("callsTotal").get_to(r.usage_info.get_sensor.calls_total);
            get_sensor.at("callsThisPeriod").get_to(r.usage_info.get_sensor.calls_this_period);
            get_sensor.at("lastCalled").get_to(r.usage_info.get_sensor.last_called);
        }

        auto get_config = usage_info.at("getConfig");
        if (!get_config.empty()) {
            get_config.at("callsTotal").get_to(r.usage_info.get_config.calls_total);
            get_config.at("callsThisPeriod").get_to(r.usage_info.get_config.calls_this_period);
            get_config.at("lastCalled").get_to(r.usage_info.get_config.last_called);
        }

        auto get_status = usage_info.at("getStatus");
        if (!get_status.empty()) {
            get_status.at("callsTotal").get_to(r.usage_info.get_status.calls_total);
            get_status.at("callsThisPeriod").get_to(r.usage_info.get_status.calls_this_period);
            get_status.at("lastCalled").get_to(r.usage_info.get_status.last_called);
        }

        auto post_stream = usage_info.at("postStream");
        if (!post_stream.empty()) {
            post_stream.at("callsTotal").get_to(r.usage_info.post_stream.calls_total);
            post_stream.at("callsThisPeriod").get_to(r.usage_info.post_stream.calls_this_period);
            post_stream.at("lastCalled").get_to(r.usage_info.post_stream.last_called);
            post_stream.at("samplesTotal").get_to(r.usage_info.post_stream.samples_total);
            post_stream.at("samplesThisPeriod").get_to(r.usage_info.post_stream.samples_this_period);
        }
    }
}
