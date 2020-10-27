#include "amber_sdk.h"
#include <iostream>

int main(int argc, char *argv[]) {

    // set up handler
    amber_sdk *sdk = new amber_sdk();

    // list all sensors
    amber_models::list_sensors_response list_sensors_response;
    if (sdk->list_sensors(list_sensors_response)) {
        list_sensors_response.dump();
    } else {
        std::cout << "error: " << sdk->last_error << "\n";
    }
}

