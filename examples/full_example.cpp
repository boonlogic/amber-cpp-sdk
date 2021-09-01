#include "amber_sdk.h"
#include <iostream>
#include <sstream>
#include <fstream>

int main(int argc, char *argv[]) {

    bool sensor_created = false;
    std::string my_sensor;

    bool verify = true;

    for (int arg = 1; arg < argc; arg++) {
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
            if (!help) {
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
    amber_sdk *amber;
    try {
        amber = new amber_sdk();
    } catch (amber_except &e) {
        std::cout << e.what() << "\n";
        exit(1);
    }
    if (verify == false) {
        amber->verify_certificate(verify);
    }

    // get version
    amber_models::get_version_response get_version_response;
    if (amber->get_version(get_version_response)) {
        get_version_response.dump();
    } else {
        std::cout << "error: " << amber->last_error << "\n";
    }

    if (my_sensor.empty()) {
        // no sensor specified, create one
        std::string sensor_label = "fancy-sensor-6";
        std::cout << "creating sensor " << sensor_label << "\n";
        amber_models::create_sensor_response create_sensor_response;
        if (amber->create_sensor(create_sensor_response, &sensor_label)) {
            create_sensor_response.dump();
            my_sensor = create_sensor_response.sensorId;
            sensor_created = true;
        } else {
            std::cout << "error: " << amber->last_error << "\n";
        }
    }

    // list all sensors
    amber_models::list_sensors_response list_sensors_response;
    if (amber->list_sensors(list_sensors_response)) {
        list_sensors_response.dump();
    } else {
        std::cout << "error: " << amber->last_error << "\n";
    }

    // get sensor
    amber_models::get_sensor_response get_sensor_response;
    if (amber->get_sensor(get_sensor_response, my_sensor)) {
        get_sensor_response.dump();
    } else {
        std::cout << "error: " << amber->last_error << "\n";
    }

    // update a sensor
    amber_models::update_sensor_response update_sensor_response;
    std::string new_label = "fancy-sensor-7";
    if (amber->update_sensor(update_sensor_response, my_sensor, new_label)) {
        update_sensor_response.dump();
    } else {
        std::cout << "error: " << amber->last_error << "\n";
    }

    // configure a sensor
    amber_models::configure_sensor_response configure_sensor_response;
    if (amber->configure_sensor(configure_sensor_response, my_sensor, 1, 25)) {
        configure_sensor_response.dump();
    } else {
        std::cout << "error: " << amber->last_error << "\n";
    }

    // get the configuration
    amber_models::get_config_response get_config_response;
    if (amber->get_config(get_config_response, my_sensor)) {
        get_config_response.dump();
    } else {
        std::cout << "error: " << amber->last_error << "\n";
    }

    // pretrain a sensor
    amber_models::pretrain_sensor_response pretrain_sensor_response;
    // Read in pretrain data //
    std::string traindata, line;
    std::ifstream myFile("./examples/pretrain-data.csv");
    float val;
    while(std::getline(myFile, line)) {
        std::stringstream ss(line);
        while(ss >> val) {
            traindata = traindata + std::to_string(val) + ",";
            if(ss.peek() == ',') ss.ignore();
        }
    }
    traindata.pop_back();
    myFile.close();
    ///////////////////////////

    bool autotuneConfig = true;
    if (amber->pretrain_sensor(pretrain_sensor_response, my_sensor, traindata, autotuneConfig)) {
        pretrain_sensor_response.dump();
    } else {
        std::cout << "error: " << amber->last_error << "\n";
    }

    // get pretrain status
    amber_models::get_pretrain_response get_pretrain_response;
    if (amber->get_pretrain(get_pretrain_response, my_sensor)) {
        get_pretrain_response.dump();
    } else {
        std::cout << "error: " << amber->last_error << "\n";
    }

    // stream data to a sensor
    amber_models::stream_sensor_response stream_sensor_response;
    std::string csvdata = "0.1001,0.1002,0.2002,0.1111";
    if (amber->stream_sensor(stream_sensor_response, my_sensor, csvdata)) {
        stream_sensor_response.dump();
    } else {
        std::cout << "error: " << amber->last_error << "\n";
    }

    // get sensor status
    amber_models::get_status_response get_status_response;
    if (amber->get_status(get_status_response, my_sensor)) {
        get_status_response.dump();
    } else {
        std::cout << "error: " << amber->last_error << "\n";
    }

    // get root-cause by idlist
    amber_models::get_root_cause_response get_root_cause_response;
    std::string idlist = "[1]";
    if (amber->get_root_cause_by_idlist(get_root_cause_response, idlist, my_sensor)) {
        get_root_cause_response.dump();
    } else {
        std::cout << "error: " << amber->last_error << "\n";
    }

    // get root-cause by patternlist
    std::string patternlist = "[[1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5],[1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2]]";
    if (amber->get_root_cause_by_patternlist(get_root_cause_response, patternlist, my_sensor)) {
        get_root_cause_response.dump();
    } else {
        std::cout << "error: " << amber->last_error << "\n";
    }

    // delete a sensor
    if (sensor_created && !amber->delete_sensor(my_sensor)) {
        std::cout << "error: " << amber->last_error << "\n";
    }
}
