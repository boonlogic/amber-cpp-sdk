#ifndef AMBER_CPP_SDK_MODELS_H
#define AMBER_CPP_SDK_MODELS_H

#include <string>
#include <iostream>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace amber_models {

    class license_entry {
    public:
        std::string username;
        std::string password;
        std::string server;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(license_entry, username, password, server)

        void dump() {
            std::string redacted = "********";
            json j = license_entry{username, redacted, server};
            std::cout << j.dump(4) << "\n";
        }
    };

    class auth_request {
    public:
        std::string username;
        std::string password;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(auth_request, username, password)
    };

    class auth_response {
    public:
        std::string idToken;
        std::string expiresIn;
        std::string refreshToken;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(auth_response, idToken, expiresIn, refreshToken)

        void dump() {
            json j = license_entry{idToken, expiresIn, refreshToken};
            std::cout << j.dump(4) << "\n";
        }
    };

    class sensor_instance {
    public:
        std::string sensorId;
        std::string tenantId;
        std::string label;

        // optional labels require custom to_json/from_json methods
        friend void to_json(json &j, const sensor_instance &r) {
            j = json{{"label",    r.label},
                     {"sensorId", r.sensorId},
                     {"tenantId", r.tenantId}
            };
        }

        friend void from_json(const json &j, sensor_instance &r) {
            try { j.at("label").get_to(r.label); } catch (nlohmann::detail::out_of_range) {} // not required
            j.at("sensorId").get_to(r.sensorId);
            j.at("tenantId").get_to(r.tenantId);
        }

        void dump() {
            json j = json{{"label",    label},
                          {"sensorId", sensorId},
                          {"tenantId", tenantId}
            };
            std::cout << j.dump(4) << "\n";
        }
    };

    class list_sensors_response {
    public:
        std::vector<sensor_instance> sensors;

        friend void to_json(json &j, const list_sensors_response &r) {
            for (const auto &sensor: r.sensors) {
                j.push_back(sensor);
            }
        }

        friend void from_json(const json &j, list_sensors_response &r) {
            for (const auto &sensor: j) {
                r.sensors.push_back(sensor.get<amber_models::sensor_instance>());
            }
        }

        void dump() {
            json j = sensors;
            std::cout << j.dump(4) << "\n";
        }
    };

    // post sensor request
    class create_sensor_request {
    public:

        create_sensor_request(std::string label) {
            this->label = label;
        }

        std::string label;

        // optional label requires custom to_json/from_json methods
        friend void to_json(json &j, const create_sensor_request &r) {
            j = json{{"label", r.label}
            };
        }

        friend void from_json(const json &j, create_sensor_request &r) {
            try { j.at("label").get_to(r.label); } catch (nlohmann::detail::out_of_range) {} // not required
        }

        void dump() {
            json j = json{{"label", label}
            };
            std::cout << j.dump(4) << "\n";
        }
    };

    // post sensor response
    class create_sensor_response {
    public:
        std::string sensorId;
        std::string tenantId;
        std::string label;

        // optional label requires custom to_json/from_json methods
        friend void to_json(json &j, const create_sensor_response &r) {
            j = json{{"sensorId", r.sensorId},
                     {"tenantId", r.tenantId},
                     {"label",    r.label}
            };
        }

        friend void from_json(const json &j, create_sensor_response &r) {
            try { j.at("label").get_to(r.label); } catch (nlohmann::detail::out_of_range) {} // not required
            j.at("sensorId").get_to(r.sensorId);
            j.at("tenantId").get_to(r.tenantId);
        }

        void dump() {
            json j = json{{"sensorId", sensorId},
                          {"tenantId", tenantId},
                          {"label",    label}
            };
            std::cout << j.dump(4) << "\n";
        }
    };

    // get sensor response
    struct usage_element {
        uint64_t callsTotal;
        uint64_t callsThisPeriod;
        std::string lastCalled;
    };

    struct post_stream_element : usage_element {
        uint64_t samplesTotal;
        uint64_t samplesThisPeriod;
    };

    class get_sensor_response {
    public:
        std::string sensorId;
        std::string tenantId;
        std::string label;
        struct usage_info {
            usage_element putSensor;
            usage_element postConfig;
            usage_element getSensor;
            usage_element getConfig;
            usage_element getStatus;
            post_stream_element postStream;
        } usage_info;

        void fmt_json(json &j) {
            j = json{{"sensorId", sensorId},
                     {"tenantId", tenantId},
                     {"label",    label},
                     {"usageInfo",
                                  {
                                      {"putSensor", {
                                                        {"callsTotal", usage_info.putSensor.callsTotal},
                                                        {"callsThisPeriod", usage_info.putSensor.callsThisPeriod},
                                                        {"lastCalled", usage_info.putSensor.lastCalled}
                                                    }
                                      },
                                      {"postConfig", {
                                                         {"callsTotal", usage_info.postConfig.callsTotal},
                                                         {"callsThisPeriod", usage_info.postConfig.callsThisPeriod},
                                                         {"lastCalled", usage_info.postConfig.lastCalled}
                                                     }
                                      },
                                      {"getSensor", {
                                                        {"callsTotal", usage_info.getSensor.callsTotal},
                                                        {"callsThisPeriod", usage_info.getSensor.callsThisPeriod},
                                                        {"lastCalled", usage_info.getSensor.lastCalled}
                                                    }
                                      },
                                      {"getConfig", {
                                                        {"callsTotal", usage_info.getConfig.callsTotal},
                                                        {"callsThisPeriod", usage_info.getConfig.callsThisPeriod},
                                                        {"lastCalled", usage_info.getConfig.lastCalled}
                                                    }
                                      },
                                      {"getStatus", {
                                                        {"callsTotal", usage_info.getStatus.callsTotal},
                                                        {"callsThisPeriod", usage_info.getStatus.callsThisPeriod},
                                                        {"lastCalled", usage_info.getStatus.lastCalled}
                                                    }
                                      },
                                      {"postStream", {
                                                         {"callsTotal", usage_info.postStream.callsTotal},
                                                         {"callsThisPeriod", usage_info.postStream.callsThisPeriod},
                                                         {"lastCalled", usage_info.postStream.lastCalled},
                                                         {"samplesTotal", usage_info.postStream.samplesTotal},
                                                         {"samplesThisPeriod", usage_info.postStream.samplesThisPeriod}
                                                     }
                                      }
                                  }
                     }
            };
        }

        friend void to_json(json &j, get_sensor_response &r) {
            r.fmt_json(j);
        }

        friend void from_json(const json &j, get_sensor_response &r) {
            j.at("sensorId").get_to(r.sensorId);
            j.at("tenantId").get_to(r.tenantId);
            try { j.at("label").get_to(r.label); } catch (nlohmann::detail::out_of_range) {} // not required
            auto usage_info = j.at("usageInfo");

            auto put_sensor = usage_info.at("putSensor");
            if (!put_sensor.empty()) {
                put_sensor.at("callsTotal").get_to(r.usage_info.putSensor.callsTotal);
                put_sensor.at("callsThisPeriod").get_to(r.usage_info.putSensor.callsThisPeriod);
                put_sensor.at("lastCalled").get_to(r.usage_info.putSensor.lastCalled);
            }

            auto post_config = usage_info.at("postConfig");
            if (!post_config.empty()) {
                post_config.at("callsTotal").get_to(r.usage_info.postConfig.callsTotal);
                post_config.at("callsThisPeriod").get_to(r.usage_info.postConfig.callsThisPeriod);
                post_config.at("lastCalled").get_to(r.usage_info.postConfig.lastCalled);
            }

            auto get_sensor = usage_info.at("getSensor");
            if (!get_sensor.empty()) {
                get_sensor.at("callsTotal").get_to(r.usage_info.getSensor.callsTotal);
                get_sensor.at("callsThisPeriod").get_to(r.usage_info.getSensor.callsThisPeriod);
                get_sensor.at("lastCalled").get_to(r.usage_info.getSensor.lastCalled);
            }

            auto get_config = usage_info.at("getConfig");
            if (!get_config.empty()) {
                get_config.at("callsTotal").get_to(r.usage_info.getConfig.callsTotal);
                get_config.at("callsThisPeriod").get_to(r.usage_info.getConfig.callsThisPeriod);
                get_config.at("lastCalled").get_to(r.usage_info.getConfig.lastCalled);
            }

            auto get_status = usage_info.at("getStatus");
            if (!get_status.empty()) {
                get_status.at("callsTotal").get_to(r.usage_info.getStatus.callsTotal);
                get_status.at("callsThisPeriod").get_to(r.usage_info.getStatus.callsTotal);
                get_status.at("lastCalled").get_to(r.usage_info.getStatus.lastCalled);
            }

            auto post_stream = usage_info.at("postStream");
            if (!post_stream.empty()) {
                post_stream.at("callsTotal").get_to(r.usage_info.postStream.callsTotal);
                post_stream.at("callsThisPeriod").get_to(r.usage_info.postStream.callsTotal);
                post_stream.at("lastCalled").get_to(r.usage_info.postStream.lastCalled);
                post_stream.at("samplesTotal").get_to(r.usage_info.postStream.samplesTotal);
                post_stream.at("samplesThisPeriod").get_to(r.usage_info.postStream.samplesThisPeriod);
            }
        }

        void dump() {
            json j;
            fmt_json(j);
            std::cout << j.dump(4) << "\n";
        }
    };

    // update_sensor_request
    class update_sensor_request {
    public:
        std::string label;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(update_sensor_request, label)
    };

    // update_sensor_response
    class update_sensor_response {
    public:
        std::string label;
        std::string sensorId;
        std::string tenantId;

        friend void to_json(json &j, const update_sensor_response &r) {
            j = json{{"label",    r.label},
                     {"sensorId", r.sensorId},
                     {"tenantId", r.tenantId}
            };
        }

        friend void from_json(const json &j, update_sensor_response &r) {
            try { j.at("label").get_to(r.label); } catch (nlohmann::detail::out_of_range) {} // not required
            j.at("sensorId").get_to(r.sensorId);
            j.at("tenantId").get_to(r.tenantId);
        }

        void dump() {
            json j = json{{"label", label}
            };
            std::cout << j.dump(4) << "\n";
        }
    };

    class configure_sensor_request {
    public:
        uint32_t featureCount;
        uint16_t streamingWindowSize;
        uint32_t samplesToBuffer;
        uint64_t learningRateNumerator;
        uint64_t learningRateDenominator;
        uint16_t learningMaxClusters;
        uint64_t learningMaxSamples;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(configure_sensor_request, featureCount, streamingWindowSize,
                                       samplesToBuffer, learningRateNumerator, learningRateDenominator,
                                       learningMaxClusters, learningMaxSamples)
    };

    class configure_sensor_response {
    public:
        uint32_t featureCount;
        uint16_t streamingWindowSize;
        uint32_t samplesToBuffer;
        uint64_t learningRateNumerator;
        uint64_t learningRateDenominator;
        uint16_t learningMaxClusters;
        uint64_t learningMaxSamples;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(configure_sensor_response, featureCount, streamingWindowSize,
                                       samplesToBuffer, learningRateNumerator, learningRateDenominator,
                                       learningMaxClusters, learningMaxSamples)

        void dump() {
            json j = json{{"featureCount",            featureCount},
                          {"streamingWindowSize",     streamingWindowSize},
                          {"samplesToBuffer",         samplesToBuffer},
                          {"learningRateNumerator",   learningRateNumerator},
                          {"learningRateDenominator", learningRateDenominator},
                          {"learningMaxClusters",     learningMaxClusters},
                          {"learningMaxSamples",      learningMaxSamples}
            };
            std::cout << j.dump(4) << "\n";
        }
    };

    class stream_sensor_request {
    public:
        std::string data;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(stream_sensor_request, data)
    };

    class stream_sensor_response {
    public:
        std::string state;
        std::string message;
        int progress;
        int clusterCount;
        int retryCount;
        int streamingWindowSize;
        std::vector<uint32_t> ID;
        std::vector<uint16_t> SI;
        std::vector<uint16_t> AD;
        std::vector<uint16_t> AH;
        std::vector<float> AM;
        std::vector<uint16_t> AW;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(stream_sensor_response, state, message, progress, clusterCount, retryCount,
                                       streamingWindowSize, ID, SI, AD, AH, AM, AW)

        void dump() {
            json j = *this;
            std::cout << j.dump(4) << "\n";
        }
    };

    struct config_features {
        float minVal;
        float maxVal;
    };

    class get_config_response {
    public:
        uint32_t featureCount;
        uint16_t streamingWindowSize;
        uint32_t samplesToBuffer;
        uint64_t learningRateNumerator;
        uint64_t learningRateDenominator;
        uint16_t learningMaxClusters;
        uint64_t learningMaxSamples;
        float percentVariation;
        std::vector<config_features> features;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(get_config_response, featureCount, streamingWindowSize, samplesToBuffer,
                                       learningRateNumerator, learningRateDenominator, learningMaxClusters,
                                       learningMaxSamples, percentVariation)

        void dump() {
            json j = *this;
            std::cout << j.dump(4) << "\n";
        }
    };

    class get_status_response {
    public:
        std::vector<float> pca;
        std::vector<uint64_t> clusterGrowth;
        std::vector<uint64_t> clusterSizes;
        std::vector<uint16_t> anomalyIndexes;
        std::vector<uint16_t> frequencyIndexes;
        std::vector<uint16_t> distanceIndexes;
        uint64_t totalInferences;
        uint16_t numClusters;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(get_status_response, pca, clusterGrowth, clusterSizes, anomalyIndexes,
                                       frequencyIndexes, distanceIndexes, totalInferences, numClusters)

        void dump() {
            json j = *this;
            std::cout << j.dump(4) << "\n";
        }
    };
};

#endif //AMBER_CPP_SDK_MODELS_H