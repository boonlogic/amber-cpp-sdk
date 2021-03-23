#include <stdio.h>
#include "amber_sdk.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

void message_and_exit(char *last_error) {
    std::cout << "error: " << last_error << "\n";
    exit(1);
}

int main(int argc, char *argv[]) {

    bool sensor_created = false;
    std::string my_sensor;

    bool verify = true;

    for (int arg = 1 ; arg < argc ; arg++) {
        std::string str(argv[arg]);
        while (str.find('-') == 0) {
            str.erase(0, 1);
        }

        if (strcasecmp("noverify", str.c_str()) == 0) {
            verify = false;
        } else {
            bool help = false;
            if (strcasecmp("help", str.c_str()) == 0) {
                help = true;
            }
            if (! help) {
                if (my_sensor.empty()) {
                    my_sensor = str;
                } else {
                    help = true;
                    std::cout << "error: unknown argument '" << str << "'\n";
                }
            }
            if (help) {
                std::cout << "usage: " << argv[0] << " [--noverify] <sensorID>\n";
                exit(1);
            }
        }
    }

    // set up handler
    amber_sdk *sdk;
    try {
        sdk = new amber_sdk();
    } catch (amber_except &e) {
        std::cout << e.what() << "\n";
        exit(1);
    }
    if (verify == false) {
        sdk->verify_certificate(verify);
    }

    if (my_sensor.empty()) {
        // no sensor specified, create one
        std::string sensor_label = "fancy-sensor-6";
        amber_models::create_sensor_response create_sensor_response;
        if (sdk->create_sensor(create_sensor_response, sensor_label)) {
            create_sensor_response.dump();
            my_sensor = create_sensor_response.sensorId;
            sensor_created = true;
        } else {
            message_and_exit(sdk->last_error);
        }
    }

    // configure the sensor
    amber_models::configure_sensor_response configure_sensor_response;
    if (sdk->configure_sensor(configure_sensor_response, my_sensor, 1, 25)) {
        configure_sensor_response.dump();
    } else {
        message_and_exit(sdk->last_error);
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
            message_and_exit(sdk->last_error);
        }
    }

    // delete a sensor
    if (sensor_created && !sdk->delete_sensor(my_sensor)) {
        message_and_exit(sdk->last_error);
    }
}
