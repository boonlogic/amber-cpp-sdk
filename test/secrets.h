#pragma once

#include "nlohmann/json.hpp"
#include "amber_sdk.h"

static nlohmann::json env_mappings = {
    {"AMBER_LICENSE_ID", "license_id"}, {"AMBER_LICENSE_FILE", "license_file"},
    {"AMBER_USERNAME", "username"},     {"AMBER_PASSWORD", "password"},
    {"AMBER_SERVER", "server"},         {"AMBER_OAUTH_SERVER", "oauth-server"}};

amber_sdk* create_amber_client();

std::string exec(const char *cmd);

nlohmann::json get_secrets();

void load_credentials_into_env();

nlohmann::json clear_env_variables();

void restore_env_variables(nlohmann::json saved_env);
