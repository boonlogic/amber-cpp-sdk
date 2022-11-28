#include "secrets.h"
#include "amber_sdk.h"
#include <fstream>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <Windows.h>
#else

#include <unistd.h>

#endif

amber_sdk *create_amber_client() {

  amber_sdk *ac;
  char *amber_license_file = getenv("AMBER_TEST_LICENSE_FILE");
  char *amber_license_id = getenv("AMBER_TEST_LICENSE_ID");
  if (!amber_license_id) {
    assert("AMBER_TEST_LICENSE_ID required in environment");
  }

  if (amber_license_file) {
    // load license profile using a local license file
    ac = new amber_sdk(amber_license_id, amber_license_file);
  } else {
    // load license profile from secrets manager
    json secrets = get_secrets();
    if (!secrets.contains(amber_license_id)) {
      assert("invalid license identifier");
    }
    setenv("AMBER_USERNAME",
           std::string(secrets[amber_license_id]["username"]).c_str(), 1);
    setenv("AMBER_PASSWORD",
           std::string(secrets[amber_license_id]["password"]).c_str(), 1);
    setenv("AMBER_SERVER",
           std::string(secrets[amber_license_id]["server"]).c_str(), 1);
    setenv("AMBER_OAUTH_SERVER",
           secrets[amber_license_id].contains("oauth-server")
               ? std::string(secrets[amber_license_id]["oauth-server"]).c_str()
               : std::string(secrets[amber_license_id]["server"]).c_str(),
           1);
    ac = new amber_sdk(NULL, NULL);
  }

  return ac;
}

std::string exec(const char *cmd) {
  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return result;
}

json get_secrets() {
  std::string raw_secrets =
      exec("aws secretsmanager get-secret-value --secret-id amber-test-users "
           "--output text --query SecretString");
  json secrets = json::parse(raw_secrets);
  return secrets;
}

json clear_env_variables() {
  json saved_profile = json::object({{"AMBER_LICENSE_FILE", NULL},
                                     {"AMBER_LICENSE_ID", NULL},
                                     {"AMBER_USERNAME", NULL},
                                     {"AMBER_PASSWORD", NULL},
                                     {"AMBER_SERVER", NULL},
                                     {"AMBER_OAUTH_SERVER", NULL}});

  for (auto it : saved_profile.items()) {
    if (getenv(it.key().c_str())) {
      saved_profile[it.key()] = std::string(getenv(it.key().c_str()));
      unsetenv(it.key().c_str());
    }
  }
  return saved_profile;
}

void restore_env_variables(json saved_env) {
  // clear out existing environment
  for (auto it : env_mappings.items()) {
    unsetenv(it.key().c_str());
  }

  // load environment from saved profile
  for (auto it : saved_env.items()) {
    if (it.value() != NULL) {
      std::string mykey = it.key();
      std::string myval = saved_env[mykey];
      setenv(mykey.c_str(), myval.c_str(), 1);
    }
  }
}