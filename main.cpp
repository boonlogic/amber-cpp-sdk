#include "sdk.h"
#include <iostream>

int main(int argc, char *argv[]) {

    std::string my_sensor = "00456a69350f72b1";

    amber_sdk *sdk = new amber_sdk();

    // list all sensors
    auto sensor_list = sdk->list_sensors();

    // create new sensor
    // auto new_sensor = sdk->create_sensor(std::string("fancy-sensor-6"));

    // get sensor
    auto get_response = sdk->get_sensor(my_sensor);

    // update a sensor
    auto update_response = sdk->update_sensor(my_sensor, "b_sensor_label");

    // configure a sensor
    auto c_sensor = sdk->configure_sensor(my_sensor, 1, 25);
}

