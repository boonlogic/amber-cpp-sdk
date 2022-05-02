#ifndef AMBER_CPP_SDK_AMBER_SDK_H
#define AMBER_CPP_SDK_AMBER_SDK_H

#include "amber_models.h"
#include "nlohmann/json.hpp"
#include <ctime>
#include <curl/curl.h>
#include <exception>
#include <stdarg.h>
#include <string>

using json = nlohmann::json;

class amber_except : public std::exception {
public:
  amber_except(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsprintf(this->buffer, fmt, args);
    va_end(args);
  };

  char *what() { return this->buffer; }

  char buffer[256];
};

typedef amber_models::PostSensorResponse create_sensor_response;
typedef amber_models::GetSensorResponse get_sensor_response;
typedef amber_models::GetSensorsResponse list_sensors_response;
typedef amber_models::PutSensorResponse update_sensor_response;
typedef amber_models::PostConfigResponse configure_sensor_response;
typedef amber_models::PutConfigResponse configure_fusion_response;
typedef amber_models::PutStreamResponse stream_fusion_response;
typedef amber_models::PostStreamResponse stream_sensor_response;
typedef amber_models::PutConfigResponse enable_learning_response;
typedef amber_models::PostPretrainResponse pretrain_sensor_response;
typedef amber_models::GetPretrainResponse get_pretrain_response;
typedef amber_models::GetStatusResponse get_status_response;
typedef amber_models::GetConfigResponse get_config_response;
typedef amber_models::Version version_response;
typedef amber_models::GetRootCauseResponse get_root_cause_response;

class amber_sdk {
public:
  explicit amber_sdk() {
    std::string cert, cainfo;
    amber_init("default", "~/.Amber.license", true, "", "");
  }
  explicit amber_sdk(const std::string &license_id,
                     const std::string &license_file) {
    std::string cert, cainfo;
    amber_init(license_id, license_file, true, cert, cainfo);
  }
  explicit amber_sdk(const std::string &license_id,
                     const std::string &license_file, bool verify_cert,
                     const std::string &cert, const std::string &cainfo) {
    amber_init(license_id, license_file, verify_cert, cert, cainfo);
  }

  ~amber_sdk();

  bool amber_init(const std::string &license_id,
                  const std::string &license_file, bool verify_cert,
                  const std::string &cert, const std::string &cainfo);

  void verify_certificate(bool verify_cert) { ssl.verify = verify_cert; }

  void set_cert(std::string cert) { ssl.cert = cert; }

  void set_cainfo(std::string cainfo) { ssl.cainfo = cainfo; }

  bool create_sensor(create_sensor_response &response,
                     std::string label = NULL);

  bool get_sensor(get_sensor_response &response, const std::string &sensor_id);

  bool list_sensors(list_sensors_response &response);

  bool update_sensor(update_sensor_response &response,
                     const std::string sensor_id, std::string &label);

  bool delete_sensor(const std::string &sensor_id);

  bool configure_sensor(configure_sensor_response &response,
                        const std::string &sensor_id,
                        uint16_t feature_count = 1,
                        uint16_t streaming_window_size = 25,
                        uint32_t samples_to_buffer = 1000,
                        uint64_t learning_rate_numerator = 0,
                        uint64_t learning_rate_denominator = 2000,
                        uint16_t learning_max_clusters = 1000,
                        uint64_t learning_max_samples = 1000000,
                        uint32_t anomaly_history_window = 10000,
                        std::vector<amber_models::FeatureConfig> features = {});

  bool configure_fusion(configure_fusion_response &response,
                        const std::string &sensor_id,
                        amber_models::PutConfigRequest request);

  bool stream_fusion(stream_fusion_response &response,
                     const std::string &sensor_id,
                     amber_models::PutStreamRequest request);

  bool stream_sensor(stream_sensor_response &response,
                     const std::string &sensor_id, std::string csvdata,
                     bool save_image = true);

  bool enable_learning(enable_learning_response &response,
                       const std::string &sensor_id,
                       uint32_t anomaly_history_window = 10000,
                       uint64_t learning_rate_numerator = 0,
                       uint64_t learning_rate_denominator = 2000,
                       uint16_t learning_max_clusters = 1000,
                       uint64_t learning_max_samples = 1000000);

  bool pretrain_sensor(pretrain_sensor_response &response,
                       const std::string &sensor_id, std::string csvdata,
                       bool autotuneConfig = true, bool block = true);

  bool get_pretrain(get_pretrain_response &response,
                    const std::string &sensor_id);

  bool get_config(get_config_response &response, const std::string &sensor_id);

  bool get_status(get_status_response &response, const std::string &sensor_id);

  bool get_version(version_response &response);

  bool get_root_cause_by_idlist(get_root_cause_response &response,
                                const std::string &sensor_id,
                                std::string id_list);

  bool get_root_cause_by_patternlist(get_root_cause_response &response,
                                     const std::string &sensor_id,
                                     std::string pattern_list);

  int last_code;
  char last_error[CURL_ERROR_SIZE];

  amber_models::license_entry license;

  void reset_last_message() {
    last_code = 0;
    last_error[0] = 0;
  }

private:
  void common_curl_opts(CURL *curl, std::string &url, struct curl_slist *hs,
                        std::string *rbufptr);

  bool authenticate(json &response);

  int get_request(std::string &slug, std::string &query_params,
                  const std::string &sensor_id, json &response);

  int post_request(std::string &slug, const std::string &sensor_id,
                   std::string &body, bool do_auth, json &response);

  int put_request(std::string &slug, const std::string &sensor_id,
                  std::string &body, json &response);

  int delete_request(std::string &slug, const std::string &sensor_id,
                     json &response);

  amber_models::PostAuth2Response auth;
  uint64_t expires_in;
  bool auth_ok;
  std::time_t auth_time;
  std::string auth_bear_header;
  std::string license_id;
  std::string license_file;

  // ssl options
  struct {
    bool verify;
    std::string cert;
    std::string cainfo;
  } ssl;
};

#endif // AMBER_CPP_SDK_AMBER_SDK_H
