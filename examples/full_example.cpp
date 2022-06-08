#include "amber_sdk.h"
#include <fstream>
#include <iostream>
#include <sstream>

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

  // get version
  get_version_response version_response;
  err = amber->get_version(version_response);
  if (!err) {
    version_response.dump();
  } else {
    err->dump();
  }

  if (my_sensor.empty()) {
    // no sensor specified, create one
    std::string sensor_label = "fancy-sensor-6";
    std::cout << "creating sensor " << sensor_label << "\n";
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

  // list all sensors
  list_sensors_response list_sensors_response;
  err = amber->list_sensors(list_sensors_response);
  if (!err) {
    list_sensors_response.dump();
  } else {
    err->dump();
  }

  // get sensor
  get_sensor_response get_sensor_response;
  err = amber->get_sensor(get_sensor_response, my_sensor);
  if (!err) {
    get_sensor_response.dump();
  } else {
    err->dump();
  }

  // update a sensor
  update_sensor_response update_sensor_response;
  std::string new_label = "fancy-sensor-7";
  err = amber->update_sensor(update_sensor_response, my_sensor, new_label);
  if (!err) {
    update_sensor_response.dump();
  } else {
    err->dump();
  }

  // configure a sensor
  configure_sensor_response configure_sensor_response;
  err = amber->configure_sensor(configure_sensor_response, my_sensor, 1, 25);
  if (!err) {
    configure_sensor_response.dump();
  } else {
    err->dump();
  }

  // get the configuration
  get_config_response get_config_response;
  err = amber->get_config(get_config_response, my_sensor);
  if (!err) {
    get_config_response.dump();
  } else {
    err->dump();
  }

  // pretrain a sensor
  pretrain_sensor_response pretrain_sensor_response;
  // Read in pretrain data //
  std::string traindata, line;
  std::ifstream myFile("./data/pretrain-data.csv");
  float val;
  while (std::getline(myFile, line)) {
    std::stringstream ss(line);
    while (ss >> val) {
      traindata += std::to_string(val) + ",";
      if (ss.peek() == ',')
        ss.ignore();
    }
  }
  traindata.pop_back();
  myFile.close();
  ///////////////////////////

  bool autotuneConfig = true;
  err = amber->pretrain_sensor(pretrain_sensor_response, my_sensor, traindata,
                               autotuneConfig);
  if (!err) {
    pretrain_sensor_response.dump();
  } else {
    err->dump();
  }

  // get pretrain status
  get_pretrain_response get_pretrain_response;
  err = amber->get_pretrain(get_pretrain_response, my_sensor);
  if (!err) {
    get_pretrain_response.dump();
  } else {
    err->dump();
  }

  // stream data to a sensor
  stream_sensor_response stream_sensor_response;
  std::string csvdata = "0.1001,0.1002,0.2002,0.1111";
  err = amber->stream_sensor(stream_sensor_response, my_sensor, csvdata);
  if (!err) {
    stream_sensor_response.dump();
  } else {
    err->dump();
  }

  // get sensor status
  get_status_response get_status_response;
  err = amber->get_status(get_status_response, my_sensor);
  if (!err) {
    get_status_response.dump();
  } else {
    err->dump();
  }

  // get root-cause by idlist
  get_root_cause_response get_root_cause_response;
  std::vector<uint16_t> idlist = {1};
  err = amber->get_root_cause_by_idlist(get_root_cause_response, my_sensor,
                                        idlist);
  if (!err) {
    get_root_cause_response.dump();
  } else {
    err->dump();
  }

  // get root-cause by patternlist
  std::vector<std::vector<uint16_t>> patternlist = {
      {1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3,
       4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5},
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}};
  err = amber->get_root_cause_by_patternlist(get_root_cause_response, my_sensor,
                                             patternlist);
  if (!err) {
    get_root_cause_response.dump();
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
