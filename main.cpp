#include "sdk.h"
#include <iostream>

int main(int argc, char *argv[]) {

    std::string my_sensor = "00456a69350f72b1";

    amber_sdk *sdk = new amber_sdk();

    // list all sensors
    amber_models::list_sensors_response list_sensors_response;
    if (sdk->list_sensors(list_sensors_response)) {
        list_sensors_response.dump();
    } else {
        std::cout << "error: " << sdk->last_error << "\n";
    }

    // create new sensor
    std::string sensor_label = "fancy-sensor-6";
#if 0
    amber_models::create_sensor_response create_sensor_response;
    if (sdk->create_sensor(create_sensor_response, sensor_label)) {
        create_sensor_response.dump();
    } else {
        std::cout << "error: " << sdk->last_error << "\n";
    }
#endif

    // get sensor
    amber_models::get_sensor_response get_sensor_response;
    if (sdk->get_sensor(get_sensor_response, my_sensor)) {
        get_sensor_response.dump();
    } else {
        std::cout << "error: " << sdk->last_error << "\n";
    }

    // update a sensor
    amber_models::update_sensor_response update_sensor_response;
    sensor_label = "fancy-sensor-7";
    if (sdk->update_sensor(update_sensor_response, my_sensor, sensor_label)) {
        update_sensor_response.dump();
    } else {
        std::cout << "error: " << sdk->last_error << "\n";
    }

    // configure a sensor
    amber_models::configure_sensor_response configure_sensor_response;
    if (sdk->configure_sensor(configure_sensor_response, my_sensor, 1, 25)) {
        configure_sensor_response.dump();
    } else {
        std::cout << "error: " << sdk->last_error << "\n";
    }

    // get the configuration
    amber_models::get_config_response get_config_response;
    if (sdk->get_config(get_config_response, my_sensor)) {
        get_config_response.dump();
    } else {
        std::cout << "error: " << sdk->last_error << "\n";
    }

    // stream data to a sensor
    amber_models::stream_sensor_response stream_sensor_response;
    std::string csvdata = "0.1001,0.1002,0.2002,0.1111";
    if (sdk->stream_sensor(stream_sensor_response, my_sensor, csvdata)) {
        stream_sensor_response.dump();
    } else {
        std::cout << "error: " << sdk->last_error << "\n";
    }

    // get sensor status
    amber_models::get_status_response get_status_response;
    if (sdk->get_status(get_status_response, my_sensor)) {
        get_status_response.dump();
    } else {
        std::cout << "error: " << sdk->last_error << "\n";
    }

    // delete a sensor
    if (!sdk->delete_sensor(my_sensor)) {
        std::cout << "error: " << sdk->last_error << "\n";
    }
}

