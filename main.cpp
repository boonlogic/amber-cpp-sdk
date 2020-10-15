#include "sdk.h"
#include <iostream>

int main(int argc, char *argv[]) {

    amber_sdk *sdk = new amber_sdk();

    // list all sensors
    auto sensor_list = sdk->list_sensors();

    // create new sensor
    // auto new_sensor = sdk->create_sensor(std::string("fancy-sensor-6"));

    // get sensor
    auto a_sensor = sdk->get_sensor(std::string("00456a69350f72b1"));
    std::cout << a_sensor << "\n";
}

