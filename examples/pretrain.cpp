#include "amber_sdk.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string>

void message_and_exit(char *last_error) {
  std::cout << "error: " << last_error << "\n";
  exit(1);
}

int main(int argc, char *argv[]) {

  error_response *err;
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
  if (!verify) {
    amber->verify_certificate(verify);
  }

  if (my_sensor.empty()) {
    // no sensor specified, create one
    std::string sensor_label = "fancy-sensor-6";
    create_sensor_response create_sensor_response;
    err = amber->create_sensor(create_sensor_response, sensor_label);
    if (!err) {
      create_sensor_response.dump();
      my_sensor = create_sensor_response.sensorId;
      sensor_created = true;
    } else {
      err->dump();
    }
  }

  // configure the sensor
  configure_sensor_response configure_sensor_response;
  err = amber->configure_sensor(configure_sensor_response, my_sensor, 1, 25,
                                10000, 10, 10000, 1000, 1000000, 10000);
  if (!err) {
    configure_sensor_response.dump();
  } else {
    err->dump();
  }

  // Read in pretrain data //
  std::ifstream t("./data/pretrainxl.csv");
  std::stringstream training;
  training << t.rdbuf();
  t.close();

  // pretrain the sensor
  pretrain_sensor_response pretrain_sensor_response;
  err = amber->pretrain_sensor_xl(pretrain_sensor_response, my_sensor,
                                  training.str(), "csv", true);
  if (!err) {
    pretrain_sensor_response.dump();
  } else {
    err->dump();
  }

  // delete a sensor
  if (sensor_created) {
    delete_sensor_response delete_sensor_response;
    err = amber->delete_sensor(delete_sensor_response, my_sensor);
    if (!err) {
      delete_sensor_response.dump();
    } else {
      err->dump();
    }
  }
}
