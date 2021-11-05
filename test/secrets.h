#pragma once

#include "nlohmann/json.hpp"

static json env_mappings = {
    {"AMBER_LICENSE_ID",   "license_id"},
    {"AMBER_LICENSE_FILE", "license_file"},
    {"AMBER_USERNAME",     "username"},
    {"AMBER_PASSWORD",     "password"},
    {"AMBER_SERVER",       "server"},
    {"AMBER_OAUTH_SERVER", "oauth-server"},
    {"AMBER_SSL_CERT",     "ssl-cert"},
    {"AMBER_SSL_VERIFY",   "ssl-verify"}
};

std::string exec(const char *cmd);

json get_secrets();

void load_credentials_into_env();

json clear_env_variables();

void restore_env_variables(json saved_env);