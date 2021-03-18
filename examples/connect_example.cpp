#include "amber_sdk.h"
#include <iostream>

int main(int argc, char *argv[]) {

    bool verify = true;

    for (int arg = 1 ; arg < argc ; arg++) {
        std::string str(argv[arg]);
        while (str.find('-') == 0) {
            str.erase(0, 1);
        }

        if (strcasecmp("noverify", str.c_str()) == 0) {
            verify = false;
        } else {
            if (strcasecmp("help", str.c_str()) == 0) {
                std::cout << "usage: " << argv[0] << " [--noverify]\n";
            } else {
                std::cout << "error: unknown argument '" << str << "'\n";
            }
            exit(1);
        }
    }

    // set up handler
    amber_sdk *sdk;
    try {
        sdk = new amber_sdk();
        sdk->verify_certificate(verify);
    } catch (amber_except &e) {
        std::cout << e.what() << "\n";
        exit(1);
    }

    // list all sensors
    amber_models::list_sensors_response list_sensors_response;
    if (sdk->list_sensors(list_sensors_response)) {
        list_sensors_response.dump();
    } else {
        std::cout << "error: " << sdk->last_error << "\n";
    }
}

