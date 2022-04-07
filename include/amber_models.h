#ifndef AMBER_CPP_SDK_AMBER_MODELS_H
#define AMBER_CPP_SDK_AMBER_MODELS_H

#include "nlohmann/json.hpp"
#include <iostream>
#include <string>

using json = nlohmann::json;

#define AMBER_DUMP()                                                           \
  void dump() {                                                                \
    json j = *this;                                                            \
    std::cout << j.dump(4) << "\n";                                            \
  }

namespace amber_models {

class license_entry {
public:
  std::string username;
  std::string password;
  std::string server;
  std::string oauth_server;

  friend void to_json(json &j, const license_entry &r) {
    j["username"] = r.username;
    j["password"] = r.password;
    j["server"] = r.server;
    if (!r.oauth_server.empty()) {
      j["oauth-server"] = r.oauth_server;
    } else {
      j["oauth-server"] = r.server;
    }
  }

  friend void from_json(const json &j, license_entry &r) {
    r.username = j.at("username");
    r.password = j.at("password");
    r.server = j.at("server");
    if (j.contains("oauth-server")) {
      r.oauth_server = j.at("oauth-server");
    } else {
      r.server = j.at("server");
    }
  }

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

  AMBER_DUMP()
};

class auth_response {
public:
  std::string idToken;
  std::string expiresIn;
  std::string refreshToken;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(auth_response, idToken, expiresIn,
                                 refreshToken)

  AMBER_DUMP()
};

class sensor_instance {
public:
  std::string sensorId;
  std::string label;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(sensor_instance, sensorId, label)

  AMBER_DUMP()
};

class list_sensors_response {
public:
  std::vector<sensor_instance> sensors;

  friend void to_json(json &j, const list_sensors_response &r) {
    for (const auto &sensor : r.sensors) {
      j.push_back(sensor);
    }
  }

  friend void from_json(const json &j, list_sensors_response &r) {
    for (const auto &sensor : j) {
      r.sensors.push_back(sensor.get<amber_models::sensor_instance>());
    }
  }

  AMBER_DUMP()
};

// post sensor request
class create_sensor_request {
public:
  explicit create_sensor_request(std::string &label) { this->label = label; }

  std::string label;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(create_sensor_request, label)

  AMBER_DUMP()
};

// post sensor response
class create_sensor_response {
public:
  std::string sensorId;
  std::string label;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(create_sensor_response, sensorId, label)

  AMBER_DUMP()
};

// get sensor response
struct usage_element {
  uint64_t callsTotal;
  uint64_t callsThisPeriod;
  std::string lastCalled;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(usage_element, callsTotal, callsThisPeriod,
                                 lastCalled)

  AMBER_DUMP()
};

struct post_stream_element {
  uint64_t callsTotal;
  uint64_t callsThisPeriod;
  std::string lastCalled;
  uint64_t samplesTotal;
  uint64_t samplesThisPeriod;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(post_stream_element, callsTotal, lastCalled,
                                 callsThisPeriod, samplesTotal,
                                 samplesThisPeriod)

  AMBER_DUMP()
};

struct usage_info {
  usage_element putSensor;
  usage_element postConfig;
  usage_element getSensor;
  usage_element getConfig;
  usage_element getStatus;
  post_stream_element postStream;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(usage_info, putSensor, postConfig, getSensor,
                                 getConfig, getStatus, postStream)
};

class get_sensor_response {
public:
  std::string sensorId;
  std::string label;
  usage_info usageInfo;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(get_sensor_response, sensorId, label,
                                 usageInfo)

  AMBER_DUMP()
};

// update_sensor_request
class update_sensor_request {
public:
  std::string label;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(update_sensor_request, label)

  AMBER_DUMP()
};

// update_sensor_response
class update_sensor_response {
public:
  std::string sensorId;
  std::string label;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(update_sensor_response, label, sensorId)

  AMBER_DUMP()
};

struct config_features {
  float minVal;
  float maxVal;
  std::string label;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(config_features, minVal, maxVal, label)

  AMBER_DUMP()
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
  uint32_t anomalyHistoryWindow;
  std::vector<config_features> features;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(configure_sensor_request, featureCount,
                                 streamingWindowSize, samplesToBuffer,
                                 learningRateNumerator, learningRateDenominator,
                                 learningMaxClusters, learningMaxSamples,
                                 anomalyHistoryWindow, features)

  AMBER_DUMP()
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
  uint32_t anomalyHistoryWindow;
  std::vector<config_features> features;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(configure_sensor_response, featureCount,
                                 streamingWindowSize, samplesToBuffer,
                                 learningRateNumerator, learningRateDenominator,
                                 learningMaxClusters, learningMaxSamples,
                                 anomalyHistoryWindow, features)

  AMBER_DUMP()
};

class stream_sensor_request {
public:
  bool saveImage;
  std::string data;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(stream_sensor_request, saveImage, data)

  AMBER_DUMP()
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

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(stream_sensor_response, state, message,
                                 progress, clusterCount, retryCount,
                                 streamingWindowSize, ID, SI, AD, AH, AM, AW)

  AMBER_DUMP()
};

class pretrain_sensor_request {
public:
  std::string data;
  bool autotuneConfig;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(pretrain_sensor_request, data, autotuneConfig)

  AMBER_DUMP()
};

class pretrain_sensor_response {
public:
  std::string state;
  std::string message;

  friend void to_json(json &j, const pretrain_sensor_response &r) {
    j["state"] = r.state;
    if (!r.message.empty()) {
      j["message"] = r.message;
    }
  }

  friend void from_json(const json &j, pretrain_sensor_response &r) {
    r.state = j.at("state");
    if (j.contains("message")) {
      r.message = j.at("message");
    }
  }

  AMBER_DUMP()
};

class get_pretrain_response {
public:
  std::string state;
  std::string message;

  friend void to_json(json &j, const get_pretrain_response &r) {
    j["state"] = r.state;
    if (!r.message.empty()) {
      j["message"] = r.message;
    }
  }

  friend void from_json(const json &j, get_pretrain_response &r) {
    r.state = j.at("state");
    if (j.contains("message")) {
      r.message = j.at("message");
    }
  }

  AMBER_DUMP()
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
  uint32_t anomalyHistoryWindow;
  float percentVariation;

  std::vector<config_features> features;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(get_config_response, featureCount,
                                 streamingWindowSize, samplesToBuffer,
                                 learningRateNumerator, learningRateDenominator,
                                 learningMaxClusters, learningMaxSamples,
                                 anomalyHistoryWindow, percentVariation,
                                 features)

  AMBER_DUMP()
};

class get_status_response {
public:
  std::vector<std::vector<float>> pca;
  std::vector<uint64_t> clusterGrowth;
  std::vector<uint64_t> clusterSizes;
  std::vector<uint16_t> anomalyIndexes;
  std::vector<uint16_t> frequencyIndexes;
  std::vector<uint16_t> distanceIndexes;
  uint64_t totalInferences;
  uint16_t numClusters;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(get_status_response, pca, clusterGrowth,
                                 clusterSizes, anomalyIndexes, frequencyIndexes,
                                 distanceIndexes, totalInferences, numClusters)

  AMBER_DUMP()
};

class get_version_response {
public:
  std::string builder;
  std::string expertApi;
  std::string expertCommon;
  std::string nanoSecure;
  std::string release;
  std::string swaggerUi;
  std::string apiVersion;

  friend void to_json(json &j, const get_version_response &r) {
    j["builder"] = r.builder;
    j["expert-api"] = r.expertApi;
    j["expert-common"] = r.expertCommon;
    j["nano-secure"] = r.nanoSecure;
    j["release"] = r.release;
    j["swagger-ui"] = r.swaggerUi;
    j["api-version"] = r.apiVersion;
  }

  friend void from_json(const json &j, get_version_response &r) {
    r.builder = j.at("builder");
    r.expertApi = j.at("expert-api");
    r.expertCommon = j.at("expert-common");
    r.nanoSecure = j.at("nano-secure");
    r.release = j.at("release");
    r.swaggerUi = j.at("swagger-ui");
    r.apiVersion = j.at("api-version");
  }

  AMBER_DUMP()
};

class get_root_cause_response {
public:
  std::vector<std::vector<float>> root_cause;

  friend void to_json(json &j, const get_root_cause_response &r) {
    for (const auto &root_cause : r.root_cause) {
      j.push_back(root_cause);
    }
  }

  friend void from_json(const json &j, get_root_cause_response &r) {
    for (const auto &root_cause : j) {
      r.root_cause.push_back(root_cause.get<std::vector<float>>());
    }
  }

  AMBER_DUMP()
};

}; // namespace amber_models

#endif // AMBER_CPP_SDK_AMBER_MODELS_H