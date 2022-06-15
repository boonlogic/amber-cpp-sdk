#ifndef AMBER_CPP_SDK_AMBER_SDK_H
#define AMBER_CPP_SDK_AMBER_SDK_H

#include "amber_models.h"
#include "nlohmann/json.hpp"
#include <cstdarg>
#include <ctime>
#include <curl/curl.h>
#include <exception>
#include <string>
#include <utility>

using json = nlohmann::json;

class amber_except : public std::exception {
public:
  explicit amber_except(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsprintf(this->buffer, fmt, args);
    va_end(args);
  };

  char *what() { return this->buffer; }

  char buffer[256]{};
};

typedef amber_models::PostSensorResponse create_sensor_response;
typedef amber_models::Error delete_sensor_response;
typedef amber_models::GetSensorResponse get_sensor_response;
typedef amber_models::GetSensorsResponse list_sensors_response;
typedef amber_models::PutSensorResponse update_sensor_response;
typedef amber_models::PostConfigResponse configure_sensor_response;
typedef amber_models::PutConfigResponse configure_fusion_response;
typedef amber_models::PutStreamResponse stream_fusion_response;
typedef amber_models::PostStreamResponse stream_sensor_response;
typedef amber_models::PutConfigResponse enable_learning_response;
typedef amber_models::PostPretrainResponse pretrain_sensor_response;
typedef amber_models::PostOutageResponse post_outage_response;
typedef amber_models::GetPretrainResponse get_pretrain_response;
typedef amber_models::GetStatusResponse get_status_response;
typedef amber_models::GetConfigResponse get_config_response;
typedef amber_models::Version get_version_response;
typedef amber_models::GetRootCauseResponse get_root_cause_response;
typedef amber_models::Error error_response;

class sdk_request {
public:
  std::string operation;
  std::string slug;
  std::string body;
  std::string query_params;
  std::map<std::string, std::string> headers;
};

class sdk_response {
public:
  int code;
  std::map<std::string, std::string> headers;
  json res;
};

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

  void set_cert(std::string cert) { ssl.cert = std::move(cert); }

  void set_cainfo(std::string cainfo) { ssl.cainfo = std::move(cainfo); }

  error_response *create_sensor(amber_models::PostSensorResponse &response,
                                std::string label);

  error_response *get_sensor(get_sensor_response &response,
                             const std::string &sensor_id);

  error_response *list_sensors(list_sensors_response &response);

  error_response *update_sensor(update_sensor_response &response,
                                std::string sensor_id, std::string &label);

  error_response *delete_sensor(delete_sensor_response &response,
                                const std::string &sensor_id);

  error_response *configure_sensor(
      configure_sensor_response &response, const std::string &sensor_id,
      uint16_t feature_count = 1, uint16_t streaming_window_size = 25,
      uint32_t samples_to_buffer = 1000, uint64_t learning_rate_numerator = 0,
      uint64_t learning_rate_denominator = 2000,
      uint16_t learning_max_clusters = 1000,
      uint64_t learning_max_samples = 1000000,
      uint32_t anomaly_history_window = 10000,
      std::vector<amber_models::FeatureConfig> features = {});

  error_response *
  configure_fusion(configure_fusion_response &response,
                   const std::string &sensor_id,
                   const amber_models::PutConfigRequest &request);

  error_response *stream_fusion(stream_fusion_response &response,
                                const std::string &sensor_id,
                                const amber_models::PutStreamRequest &request);

  error_response *stream_sensor(stream_sensor_response &response,
                                const std::string &sensor_id,
                                std::string csvdata, bool save_image = true);

  error_response *enable_learning(enable_learning_response &response,
                                  const std::string &sensor_id,
                                  uint32_t anomaly_history_window = 10000,
                                  uint64_t learning_rate_numerator = 0,
                                  uint64_t learning_rate_denominator = 2000,
                                  uint16_t learning_max_clusters = 1000,
                                  uint64_t learning_max_samples = 1000000);

  error_response *post_outage(post_outage_response &response,
                              const std::string &sensor_id);

  error_response *pretrain_sensor(pretrain_sensor_response &response,
                                  const std::string &sensor_id,
                                  std::string csvdata,
                                  bool autotuneConfig = true,
                                  bool blocking = true);

  error_response *pretrain_sensor_xl(pretrain_sensor_response &response,
                                     const std::string &sensor_id,
                                     std::string csvdata,
                                     bool autotuneConfig = true,
                                     bool blocking = true);

  error_response *get_pretrain(get_pretrain_response &response,
                               const std::string &sensor_id);

  error_response *get_config(get_config_response &response,
                             const std::string &sensor_id);

  error_response *get_status(get_status_response &response,
                             const std::string &sensor_id);

  error_response *get_version(get_version_response &response);

  error_response *get_root_cause_by_idlist(get_root_cause_response &response,
                                           const std::string &sensor_id,
                                           std::vector<uint16_t> id_list);

  error_response *get_root_cause_by_patternlist(
      get_root_cause_response &response, const std::string &sensor_id,
      std::vector<std::vector<uint16_t>> pattern_list);

  amber_models::license_entry license;

private:
  void call_api(sdk_request &req, sdk_response &res, bool is_auth = false);

  bool authenticate(sdk_response &res);

  amber_models::PostAuth2Response auth;
  uint64_t expires_in{};
  bool auth_ok{};
  std::time_t auth_time{};
  std::string auth_bear_header;
  std::string license_id;
  std::string license_file;

  // ssl options
  struct {
    bool verify{};
    std::string cert;
    std::string cainfo;
  } ssl;
};

static std::map<std::string, std::string>
parse_headers(const std::string &header);
static std::string base64_encode(const unsigned char *in, uint64_t len);
static std::string join_uint16_vec(std::vector<uint16_t> const &v);
static std::string
join_vec_uint16_vec(std::vector<std::vector<uint16_t>> const &v);

#endif // AMBER_CPP_SDK_AMBER_SDK_H
