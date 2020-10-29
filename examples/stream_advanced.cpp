#include <stdio.h>
#include "amber_sdk.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

int main(int argc, char *argv[]) {

    std::string my_sensor;

    // set up handler
    amber_sdk *sdk;
    try {
        sdk = new amber_sdk();
    } catch (amber_except &e) {
        std::cout << e.what() << "\n";
        exit(1);
    }

    if (argc > 1) {
        // use sensor specified as argument
        my_sensor = argv[1];
    } else {
        // no sensor specified, create one
        std::string sensor_label = "fancy-sensor-6";
        amber_models::create_sensor_response create_sensor_response;
        if (sdk->create_sensor(create_sensor_response, sensor_label)) {
            create_sensor_response.dump();
            my_sensor = create_sensor_response.sensorId;
        } else {
            std::cout << "error: " << sdk->last_error << "\n";
        }
    }

    // configure the sensor
    amber_models::configure_sensor_response configure_sensor_response;
    if (sdk->configure_sensor(configure_sensor_response, my_sensor, 1, 25)) {
        configure_sensor_response.dump();
    } else {
        std::cout << "error: " << sdk->last_error << "\n";
    }

    std::ifstream in("examples/output_current.csv");
    std::string line;
    std::string value;
    uint16_t sample_count;
    std::string csv_data;
    uint16_t batch_size = 25;
    while (getline(in, line)) {
        std::stringstream ss(line);
        while (getline(ss, value, ',')) {
            value.erase(remove_if(value.begin(), value.end(), isspace), value.end());
            if (sample_count == 0) csv_data = value;
            else csv_data = csv_data + "," + value;
            sample_count++;
            if (sample_count % batch_size == 0) {
                // stream data to a sensor
                amber_models::stream_sensor_response stream_sensor_response;
                if (sdk->stream_sensor(stream_sensor_response, my_sensor, csv_data)) {
                    stream_sensor_response.dump();
                } else {
                    std::cout << "error: " << sdk->last_error << "\n";
                }
                csv_data = "";
                sample_count = 0;
            }
        }
    }

    // send any remainder
    if (sample_count > 0) {
        // stream data to a sensor
        amber_models::stream_sensor_response stream_sensor_response;
        if (sdk->stream_sensor(stream_sensor_response, my_sensor, csv_data)) {
            stream_sensor_response.dump();
        } else {
            std::cout << "error: " << sdk->last_error << "\n";
        }
    }
}
