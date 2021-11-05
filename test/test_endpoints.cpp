#include "amber_sdk.h"
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include "secrets.h"

#ifdef _WIN32
#include <Windows.h>
#endif

namespace {
    class endpoints : public testing::Test {
    protected:
        static amber_sdk *amber;
        static std::string sensor_id;

        static void SetUpTestSuite() {
            json saved_env = clear_env_variables();
            load_credentials_into_env();
            amber = new amber_sdk(NULL, NULL);
        }

        static void TearDownTestSuite() {
            delete amber;
        }

        static void set_sid(std::string sid) {
            sensor_id = sid;
        }

        static std::string get_sid() {
            return sensor_id;
        }
    };

    amber_sdk *endpoints::amber = nullptr;
    std::string endpoints::sensor_id = "";

    TEST_F(endpoints, CreateSensor) {
        amber_models::create_sensor_response response;
        std::string sensor_label = "test-sensor-cpp";
        ASSERT_TRUE(amber->create_sensor(response, &sensor_label));
        endpoints::set_sid(response.sensorId);
    }

    TEST_F(endpoints, UpdateLabel) {
        amber_models::update_sensor_response update_sensor_response;
        std::string label = "test-sensor-cpp";
        ASSERT_TRUE(amber->update_sensor(update_sensor_response, endpoints::get_sid(), label));
        EXPECT_EQ(update_sensor_response.label, "test-sensor-cpp");

        // set a label that identifies the sensor as being created by this test
        std::string sdk_label = "test-sensor-cpp-sdk";
        ASSERT_TRUE(amber->update_sensor(update_sensor_response, sensor_id, sdk_label));
        EXPECT_EQ(update_sensor_response.label, "test-sensor-cpp-sdk");
    }

    TEST_F(endpoints, UpdateLabelNegative) {
        amber_models::update_sensor_response update_sensor_response;
        std::string label = "new-label";
        std::string bad_sensor_id = "bogus-sensor-id";
        ASSERT_FALSE(amber->update_sensor(update_sensor_response, bad_sensor_id, label));
    }

    TEST_F(endpoints, GetSensor) {
        amber_models::get_sensor_response response;
        ASSERT_TRUE(amber->get_sensor(response, sensor_id));
        EXPECT_EQ(response.label, "test-sensor-cpp-sdk");
        EXPECT_EQ(response.sensorId, endpoints::get_sid());
    }

    TEST_F(endpoints, GetSensorNegative) {
        amber_models::get_sensor_response response;
        std::string bad_sensor_id = "bogus-sensor-id";
        ASSERT_FALSE(amber->get_sensor(response, bad_sensor_id));
    }

    TEST_F(endpoints, GetVersion) {
        try {
            amber_models::get_version_response get_version_response;
            amber->get_version(get_version_response);
            EXPECT_NE(std::string(get_version_response.builder), "");
        } catch (amber_except &e) {
            ASSERT_TRUE(false) << e.what();
        }
    }

    TEST_F(endpoints, ListSensors) {
        amber_models::list_sensors_response response;
        ASSERT_TRUE(amber->list_sensors(response));
        bool found = false;
        for (int i = 0; i < response.sensors.size(); i++) {
            if (std::string(response.sensors[i].sensorId).compare(sensor_id)) {
                found = true;
            }
        }
        if (!found) {
            FAIL() << "test sensor ID not found";
        }
    }

    TEST_F(endpoints, ConfigureSensor) {
        amber_models::configure_sensor_response expected;
        expected.anomalyHistoryWindow = 1000;
        expected.featureCount = 1;
        expected.streamingWindowSize = 25;
        expected.samplesToBuffer = 1000;
        expected.learningRateNumerator = 10;
        expected.learningRateDenominator = 10000;
        expected.learningMaxClusters = 1000;
        expected.learningMaxSamples = 1000000;

        amber_models::configure_sensor_response response;
        ASSERT_TRUE(amber->configure_sensor(response, endpoints::get_sid(), 1, 25, 1000,
                                            10, 10000, 1000, 1000000, 1000));

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
        amber_models::configure_sensor_response response;
        std::string badSensor = "bogus-sensor";
        ASSERT_FALSE(amber->configure_sensor(response, badSensor, 1, 25, 1000,
                                             10, 10000, 1000, 1000000, 1000));

        // invalid feature_count
        ASSERT_FALSE(amber->configure_sensor(response, sensor_id, -1, 25, 1000,
                                             10, 10000, 1000, 1000000, 1000));
        // invalid streaming window size
        ASSERT_FALSE(amber->configure_sensor(response, sensor_id, 1, -1, 1000,
                                             10, 10000, 1000, 1000000, 1000));

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
        amber_models::get_config_response expected;
        expected.anomalyHistoryWindow = 1000;
        expected.featureCount = 1;
        expected.streamingWindowSize = 25;
        expected.samplesToBuffer = 1000;
        expected.learningRateNumerator = 10;
        expected.learningRateDenominator = 10000;
        expected.learningMaxClusters = 1000;
        expected.learningMaxSamples = 1000000;
        expected.percentVariation = 0.05;

        amber_models::get_config_response response;
        ASSERT_TRUE(amber->get_config(response, endpoints::get_sid()));

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
        amber_models::get_config_response response;
        std::string bad_sensor_id = "bogus-sensor-id";
        ASSERT_FALSE(amber->get_config(response, bad_sensor_id));
    }

    TEST_F(endpoints, GetPretrainNoneState) {
        amber_models::get_pretrain_response response;
        ASSERT_TRUE(amber->get_pretrain(response, endpoints::get_sid()));
        EXPECT_EQ(response.state, "None");
    }

    TEST_F(endpoints, GetPretrainNegative) {
        amber_models::get_pretrain_response response;
        std::string bad_sensor_id = "bogus-sensor-id";
        ASSERT_FALSE(amber->get_pretrain(response, bad_sensor_id));
    }

    TEST_F(endpoints, PretrainSensor) {
        amber_models::pretrain_sensor_response response;
        // Read in pretrain data //
        std::string traindata, line;
        std::ifstream myFile("./examples/pretrain-data.csv");
        float val;
        while (std::getline(myFile, line)) {
            std::stringstream ss(line);
            while (ss >> val) {
                traindata = traindata + std::to_string(val) + ",";
                if (ss.peek() == ',') ss.ignore();
            }
        }
        traindata.pop_back();
        myFile.close();
        ASSERT_TRUE(amber->pretrain_sensor(response, sensor_id, traindata));
        EXPECT_TRUE(response.state == "Pretraining" || response.state == "Pretrained");

        amber_models::get_pretrain_response get_response;
        while (true) {
            amber->get_pretrain(get_response, sensor_id);
            if (get_response.state == "Pretrained") {
                break;
            } else {
                sleep(5);
            }
        }
        EXPECT_EQ(get_response.state, "Pretrained");
    }

    TEST_F(endpoints, PretrainSensorNegative) {
        amber_models::pretrain_sensor_response response;
        std::string bad_sensor_id = "bogus-sensor-id";
        std::string csvdata = "1,2,3,4,5";
        ASSERT_FALSE(amber->pretrain_sensor(response, bad_sensor_id, csvdata));

        // not enough data to fill the buffer
        ASSERT_FALSE(amber->pretrain_sensor(response, sensor_id, csvdata, false));
    }

    TEST_F(endpoints, GetStatus) {
        amber_models::get_status_response response;
        ASSERT_TRUE(amber->get_status(response, endpoints::get_sid()));
        EXPECT_TRUE(response.pca.size() != 0);
        EXPECT_EQ(response.numClusters, 582);
    }

    TEST_F(endpoints, GetStatusNegative) {
        amber_models::get_status_response response;
        std::string bad_sensor_id = "bogus-sensor-id";
        ASSERT_FALSE(amber->get_status(response, bad_sensor_id));
    }

    TEST_F(endpoints, StreamSensor) {
        amber_models::stream_sensor_response response;
        std::string csvdata = "1,2,3";
        ASSERT_TRUE(amber->stream_sensor(response, endpoints::get_sid(), csvdata));
        EXPECT_EQ(response.state, "Monitoring");
        EXPECT_EQ(response.message, "");
        EXPECT_EQ(response.progress, 0);
        EXPECT_EQ(response.clusterCount, 582);
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
        amber_models::stream_sensor_response response;
        std::string bad_sensor_id = "bogus-sensor-id";
        std::string csvdata = "1";
        ASSERT_FALSE(amber->stream_sensor(response, bad_sensor_id, csvdata));

        csvdata = "1,2,&,4";
        ASSERT_FALSE(amber->stream_sensor(response, endpoints::get_sid(), csvdata));
    }

    TEST_F(endpoints, GetRootCause) {
        amber_models::get_root_cause_response response;
        std::string patternlist = "[[1,1,1,1,2,3,1,1,1,1,2,3,1,1,1,1,2,3,1,1,1,1,2,3,1]]";
        ASSERT_TRUE(amber->get_root_cause_by_patternlist(response, endpoints::get_sid(), patternlist));
    }

    TEST_F(endpoints, GetRootCauseNegative) {
        amber_models::get_root_cause_response response;
        std::string bad_sensor_id = "bogus-sensor-id";
        std::string idlist = "[1]";
        ASSERT_FALSE(amber->get_root_cause_by_idlist(response, bad_sensor_id, idlist));

        // invalid format
        idlist = "1";
        EXPECT_THROW(amber->get_root_cause_by_idlist(response, sensor_id, idlist), amber_except);

        std::string patternlist = "1,2,3";
        EXPECT_THROW(amber->get_root_cause_by_idlist(response, sensor_id, patternlist), amber_except);
    }

    TEST_F(endpoints, DeleteSensor) {
        ASSERT_TRUE(amber->delete_sensor(endpoints::get_sid()));
    }

    TEST_F(endpoints, DeleteSensorNegative) {
        // sensor shouldn't be there anymore
        ASSERT_FALSE(amber->delete_sensor(endpoints::get_sid()));
    }
}