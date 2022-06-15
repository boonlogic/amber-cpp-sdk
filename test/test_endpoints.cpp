#include "amber_sdk.h"
#include "secrets.h"
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <utility>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace {
class endpoints : public testing::Test {
protected:
  static amber_sdk *amber;
  static std::string sensor_id;

  static void SetUpTestSuite() { amber = create_amber_client(); }

  static void TearDownTestSuite() { delete amber; }

  static void set_sid(std::string sid) { sensor_id = std::move(sid); }

  static std::string get_sid() { return sensor_id; }
};

amber_sdk *endpoints::amber = nullptr;
std::string endpoints::sensor_id;

TEST_F(endpoints, CreateSensor) {
  create_sensor_response response;
  std::string sensor_label = "test-sensor-cpp";
  ASSERT_EQ(amber->create_sensor(response, sensor_label), nullptr);
  endpoints::set_sid(response.sensorId);
}

TEST_F(endpoints, UpdateLabel) {
  update_sensor_response update_sensor_response;
  std::string label = "test-sensor-cpp";
  ASSERT_EQ(
      amber->update_sensor(update_sensor_response, endpoints::get_sid(), label),
      nullptr);
  EXPECT_EQ(update_sensor_response.label, "test-sensor-cpp");

  // set a label that identifies the sensor as being created by this test
  std::string sdk_label = "test-sensor-cpp-sdk";
  ASSERT_EQ(amber->update_sensor(update_sensor_response, sensor_id, sdk_label),
            nullptr);
  EXPECT_EQ(update_sensor_response.label, "test-sensor-cpp-sdk");
}

TEST_F(endpoints, UpdateLabelNegative) {
  update_sensor_response update_sensor_response;
  std::string label = "new-label";
  std::string bad_sensor_id = "bogus-sensor-id";
  ASSERT_NE(amber->update_sensor(update_sensor_response, bad_sensor_id, label),
            nullptr);
}

TEST_F(endpoints, GetSensor) {
  get_sensor_response response;
  ASSERT_EQ(amber->get_sensor(response, sensor_id), nullptr);
  EXPECT_EQ(response.label, "test-sensor-cpp-sdk");
  EXPECT_EQ(response.sensorId, endpoints::get_sid());
}

TEST_F(endpoints, GetSensorNegative) {
  get_sensor_response response;
  std::string bad_sensor_id = "bogus-sensor-id";
  ASSERT_NE(amber->get_sensor(response, bad_sensor_id), nullptr);
}

TEST_F(endpoints, GetVersion) {
  get_version_response get_version_response;
  auto err = amber->get_version(get_version_response);
  EXPECT_EQ(err, nullptr);
  EXPECT_NE(std::string(get_version_response.builder), "");
  EXPECT_NE(std::string(get_version_response.expert_common), "");
  EXPECT_NE(std::string(get_version_response.expert_api), "");
}

TEST_F(endpoints, ListSensors) {
  list_sensors_response response;
  ASSERT_EQ(amber->list_sensors(response), nullptr);
  bool found = false;
  for (auto &i : response.value) {
    if (std::string(i.sensorId) == sensor_id) {
      found = true;
    }
  }
  if (!found) {
    FAIL() << "test sensor ID not found";
  }
}

TEST_F(endpoints, ConfigureSensor) {
  configure_sensor_response expected;
  expected.anomalyHistoryWindow = 1000;
  expected.featureCount = 1;
  expected.streamingWindowSize = 25;
  expected.samplesToBuffer = 1000;
  expected.learningRateNumerator = 10;
  expected.learningRateDenominator = 10000;
  expected.learningMaxClusters = 1000;
  expected.learningMaxSamples = 1000000;

  configure_sensor_response response;
  ASSERT_EQ(amber->configure_sensor(response, endpoints::get_sid(), 1, 25, 1000,
                                    10, 10000, 1000, 1000000, 1000),
            nullptr);

  EXPECT_EQ(expected.anomalyHistoryWindow, response.anomalyHistoryWindow);
  EXPECT_EQ(expected.featureCount, response.featureCount);
  EXPECT_EQ(expected.streamingWindowSize, response.streamingWindowSize);
  EXPECT_EQ(expected.samplesToBuffer, response.samplesToBuffer);
  EXPECT_EQ(expected.learningRateNumerator, response.learningRateNumerator);
  EXPECT_EQ(expected.learningRateDenominator, response.learningRateDenominator);
  EXPECT_EQ(expected.learningMaxClusters, response.learningMaxClusters);
  EXPECT_EQ(expected.learningMaxSamples, response.learningMaxSamples);
}

TEST_F(endpoints, ConfigureSensorNegative) {
  configure_sensor_response response;
  std::string badSensor = "bogus-sensor";
  ASSERT_NE(amber->configure_sensor(response, badSensor, 1, 25, 1000, 10, 10000,
                                    1000, 1000000, 1000),
            nullptr);

  // invalid feature_count
  ASSERT_NE(amber->configure_sensor(response, sensor_id, -1, 25, 1000, 10,
                                    10000, 1000, 1000000, 1000),
            nullptr);
  // invalid streaming window size
  ASSERT_NE(amber->configure_sensor(response, sensor_id, 1, -1, 1000, 10, 10000,
                                    1000, 1000000, 1000),
            nullptr);

#if 0
        // TODO the following tests need changes with the server to put maximums on some
        // of the sizes.  For instance, passing a -1 for samples to buffer results in a
        // sample buffer of 4294966296 being passed since c++ doesn't enforce numeric
        // prototypes strictly.
        ASSERT_FALSE(amber->configure_sensor(response, sensor_id, 1, 25, -1000,
                                             10, 10000, 1000, 1000000, 1000));
        // invalid learning_rate_numerator
        ASSERT_FALSE(amber->configure_sensor(response, sensor_id, 1, 25, 1000,
                                             -10, 10000, 1000, 1000000, 1000));
        // invalid learning_rate_denominator
        ASSERT_FALSE(amber->configure_sensor(response, sensor_id, 1, 25, 1000,
                                             10, -10000, 1000, 1000000, 1000));
        // learning_max_clusters
        ASSERT_FALSE(amber->configure_sensor(response, sensor_id, 1, 25, 1000,
                                             10, 10000, -1000, 1000000, 1000));
        // learning_max_samples
        ASSERT_FALSE(amber->configure_sensor(response, sensor_id, 1, 25, 1000,
                                             10, 10000, 1000, -1000000, 1000));
        // anomaly_history_window
        ASSERT_FALSE(amber->configure_sensor(response, sensor_id, 1, 25, 1000,
                                             10, 10000, 1000, 1000000, -1000));
#endif
}

TEST_F(endpoints, GetConfig) {
  get_config_response expected;
  expected.anomalyHistoryWindow = 1000;
  expected.featureCount = 1;
  expected.streamingWindowSize = 25;
  expected.samplesToBuffer = 1000;
  expected.learningRateNumerator = 10;
  expected.learningRateDenominator = 10000;
  expected.learningMaxClusters = 1000;
  expected.learningMaxSamples = 1000000;
  expected.percentVariation = 0.05;

  get_config_response response;
  ASSERT_EQ(amber->get_config(response, endpoints::get_sid()), nullptr);

  EXPECT_EQ(expected.anomalyHistoryWindow, response.anomalyHistoryWindow);
  EXPECT_EQ(expected.featureCount, response.featureCount);
  EXPECT_EQ(expected.streamingWindowSize, response.streamingWindowSize);
  EXPECT_EQ(expected.samplesToBuffer, response.samplesToBuffer);
  EXPECT_EQ(expected.learningRateNumerator, response.learningRateNumerator);
  EXPECT_EQ(expected.learningRateDenominator, response.learningRateDenominator);
  EXPECT_EQ(expected.learningMaxClusters, response.learningMaxClusters);
  EXPECT_EQ(expected.learningMaxSamples, response.learningMaxSamples);
  EXPECT_EQ(expected.percentVariation, response.percentVariation);
}

TEST_F(endpoints, GetConfigNegative) {
  get_config_response response;
  std::string bad_sensor_id = "bogus-sensor-id";
  ASSERT_NE(amber->get_config(response, bad_sensor_id), nullptr);
}

TEST_F(endpoints, GetPretrainNoneState) {
  get_pretrain_response response;
  ASSERT_EQ(amber->get_pretrain(response, endpoints::get_sid()), nullptr);
  EXPECT_EQ(response.state, "None");
}

TEST_F(endpoints, GetPretrainNegative) {
  get_pretrain_response response;
  std::string bad_sensor_id = "bogus-sensor-id";
  EXPECT_NE(amber->get_pretrain(response, bad_sensor_id), nullptr);
}

TEST_F(endpoints, PretrainSensor) {
  pretrain_sensor_response response;
  // Read in pretrain data //
  std::string traindata, line;
  std::ifstream myFile("./data/pretrain-data.csv");
  float val;
  while (std::getline(myFile, line)) {
    std::stringstream ss(line);
    while (ss >> val) {
      traindata += std::to_string(val) + ",";
      if (ss.peek() == ',')
        ss.ignore();
    }
  }
  traindata.pop_back();
  myFile.close();
  ASSERT_EQ(amber->pretrain_sensor(response, sensor_id, traindata), nullptr);
  EXPECT_TRUE(response.state == "Pretraining" ||
              response.state == "Pretrained");

  get_pretrain_response get_response;
  while (true) {
    ASSERT_EQ(amber->get_pretrain(get_response, sensor_id), nullptr);
    if (get_response.state == "Pretrained") {
      break;
    } else {
      sleep(5);
    }
  }
  EXPECT_EQ(get_response.state, "Pretrained");
}

TEST_F(endpoints, PretrainSensorNegative) {
  pretrain_sensor_response response;
  std::string bad_sensor_id = "bogus-sensor-id";
  std::string csvdata = "1,2,3,4,5";
  ASSERT_NE(amber->pretrain_sensor(response, bad_sensor_id, csvdata), nullptr);

  // not enough data to fill the buffer
  ASSERT_NE(amber->pretrain_sensor(response, sensor_id, csvdata, false),
            nullptr);
}

TEST_F(endpoints, GetStatus) {
  get_status_response response;
  ASSERT_EQ(amber->get_status(response, endpoints::get_sid()), nullptr);
  EXPECT_TRUE(!response.pca.value.empty());
  EXPECT_EQ(response.numClusters, 581);
}

TEST_F(endpoints, GetStatusNegative) {
  get_status_response response;
  std::string bad_sensor_id = "bogus-sensor-id";
  ASSERT_NE(amber->get_status(response, bad_sensor_id), nullptr);
}

TEST_F(endpoints, StreamSensor) {
  stream_sensor_response response;
  std::string csvdata = "1,2,3";
  ASSERT_EQ(amber->stream_sensor(response, endpoints::get_sid(), csvdata),
            nullptr);
  EXPECT_EQ(response.state, "Monitoring");
  EXPECT_EQ(response.message, "");
  EXPECT_EQ(response.progress, 0);
  EXPECT_EQ(response.clusterCount, 581);
  EXPECT_EQ(response.retryCount, 0);
  EXPECT_EQ(response.streamingWindowSize, 25);
  /*
  EXPECT_EQ(!response.SI.empty());
  EXPECT_EQ(!response.AD.empty());
  EXPECT_EQ(!response.AH.empty());
  EXPECT_EQ(!response.AM.empty());
  EXPECT_EQ(!response.AW.empty());
  */

  // EXPECT_EQ(response.SI, 3);
}

TEST_F(endpoints, StreamSensorNegative) {
  stream_sensor_response response;
  std::string bad_sensor_id = "bogus-sensor-id";
  std::string csvdata = "1";
  EXPECT_NE(amber->stream_sensor(response, bad_sensor_id, csvdata), nullptr);

  csvdata = "1,2,&,4";
  EXPECT_NE(amber->stream_sensor(response, endpoints::get_sid(), csvdata),
            nullptr);
}

TEST_F(endpoints, GetRootCause) {
  get_root_cause_response response;
  std::vector<std::vector<uint16_t>> patternlist = {{1, 1, 1, 1, 2, 3, 1, 1, 1,
                                                     1, 2, 3, 1, 1, 1, 1, 2, 3,
                                                     1, 1, 1, 1, 2, 3, 1}};
  ASSERT_EQ(amber->get_root_cause_by_patternlist(response, endpoints::get_sid(),
                                                 patternlist),
            nullptr);
}

TEST_F(endpoints, GetRootCauseNegative) {
  get_root_cause_response response;
  std::string bad_sensor_id = "bogus-sensor-id";
  std::vector<uint16_t> idlist{1};
  ASSERT_NE(amber->get_root_cause_by_idlist(response, bad_sensor_id, idlist),
            nullptr);
}

TEST_F(endpoints, PostOutage) {
  post_outage_response expected;
  expected.clusterCount = 581;
  expected.message = "";
  expected.retryCount = 0;
  expected.state = "Monitoring";
  expected.streamingWindowSize = 25;
  expected.totalInferences = 31030;

  post_outage_response response;
  error_response *error = amber->post_outage(response, endpoints::get_sid());
  ASSERT_EQ(error, nullptr);
  EXPECT_EQ(expected.clusterCount, response.clusterCount);
  EXPECT_EQ(expected.message, response.message);
  EXPECT_EQ(expected.retryCount, response.retryCount);
  EXPECT_EQ(expected.state, response.state);
  EXPECT_EQ(expected.streamingWindowSize, response.streamingWindowSize);
  EXPECT_EQ(expected.totalInferences, response.totalInferences);
}

TEST_F(endpoints, PostOutageNegative) {
  post_outage_response response;
  std::string badSensor = "bogus-sensor";
  ASSERT_NE(amber->post_outage(response, badSensor), nullptr);
}

TEST_F(endpoints, EnableLearning) {
  enable_learning_response expected;
  expected.streaming.anomalyHistoryWindow = 1000;
  expected.streaming.learningRateNumerator = 10;
  expected.streaming.learningRateDenominator = 10000;
  expected.streaming.learningMaxClusters = 1000;
  expected.streaming.learningMaxSamples = 1000000;

  enable_learning_response response;
  ASSERT_EQ(amber->enable_learning(response, endpoints::get_sid(), 1000, 10,
                                   10000, 1000, 1000000),
            nullptr);

  EXPECT_EQ(expected.streaming.anomalyHistoryWindow,
            response.streaming.anomalyHistoryWindow);
  EXPECT_EQ(expected.streaming.learningRateNumerator,
            response.streaming.learningRateNumerator);
  EXPECT_EQ(expected.streaming.learningRateDenominator,
            response.streaming.learningRateDenominator);
  EXPECT_EQ(expected.streaming.learningMaxClusters,
            response.streaming.learningMaxClusters);
  EXPECT_EQ(expected.streaming.learningMaxSamples,
            response.streaming.learningMaxSamples);
}

TEST_F(endpoints, EnableLearningNegative) {
  enable_learning_response response;
  std::string badSensor = "bogus-sensor";
  ASSERT_NE(amber->enable_learning(response, badSensor, 1000, 10, 10000, 1000,
                                   1000000),
            nullptr);

  // sensor is not in learning or monitoring
  configure_sensor_response config_response;
  ASSERT_EQ(amber->configure_sensor(config_response, endpoints::get_sid(), 1,
                                    25, 1000, 10, 10000, 1000, 1000000, 1000),
            nullptr);
  ASSERT_NE(amber->enable_learning(response, endpoints::get_sid(), 1000, 10,
                                   10000, 1000, 1000000),
            nullptr);
}

TEST_F(endpoints, DeleteSensor) {
  delete_sensor_response delete_sensor_response;
  ASSERT_EQ(amber->delete_sensor(delete_sensor_response, endpoints::get_sid()),
            nullptr);
}

TEST_F(endpoints, DeleteSensorNegative) {
  // sensor shouldn't be there anymore
  delete_sensor_response delete_sensor_response;
  ASSERT_NE(amber->delete_sensor(delete_sensor_response, endpoints::get_sid()),
            nullptr);
}

} // namespace
