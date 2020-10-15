#include "sdk.h"

int main(int argc, char *argv[]) {

    amber_sdk *sdk = new amber_sdk();

    // list all sensors
    auto sensor_list = sdk->list_sensors();

    // create new sensor
    auto new_sensor = sdk->create_sensor(std::string("fancy-sensor-6"));
}

