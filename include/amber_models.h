#ifndef AMBER_CPP_SDK_AMBER_MODELS_H
#define AMBER_CPP_SDK_AMBER_MODELS_H

#include "nlohmann/json.hpp"
#include <iostream>
#include <string>

using json = nlohmann::json;

#define AMBER_DUMP()                                                           \
  void dump() {                                                                \
    json j = *this;                                                            \
    std::cout << j.dump(4) << "\n";                                            \
  }

namespace amber_models {

class license_entry {
public:
  std::string username;
  std::string password;
  std::string server;
  std::string oauth_server;

  friend void to_json(json &j, const license_entry &r) {
    j["username"] = r.username;
    j["password"] = r.password;
    j["server"] = r.server;
    if (!r.oauth_server.empty()) {
      j["oauth-server"] = r.oauth_server;
    } else {
      j["oauth-server"] = r.server;
    }
  }

  friend void from_json(const json &j, license_entry &r) {
    r.username = j.at("username");
    r.password = j.at("password");
    r.server = j.at("server");
    if (j.contains("oauth-server")) {
      r.oauth_server = j.at("oauth-server");
    } else {
      r.server = j.at("server");
    }
  }

  void dump() {
    std::string redacted = "********";
    json j = license_entry{username, redacted, server};
    std::cout << j.dump(4) << "\n";
  }
};

}; // namespace amber_models

#include "amber_gen.h"

// overrides

#endif // AMBER_CPP_SDK_AMBER_MODELS_H