#include "amber_sdk.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include "secrets.h"

#ifdef _WIN32
#include <Windows.h>
#else

#include <unistd.h>

#endif

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
    std::string raw_secrets = exec(
        "aws secretsmanager get-secret-value --secret-id amber-test-users --output text --query SecretString");
    json secrets = json::parse(raw_secrets);
    return secrets;
}

void load_credentials_into_env() {
    char *license_file = getenv("AMBER_TEST_FILE");
    if (license_file) {
        // load profile for testing using the AMBER_TEST_FILE env file
        try {
            amber_sdk *amber = new amber_sdk(NULL, license_file, false, NULL, NULL);
            setenv("AMBER_USERNAME", amber->license.username.c_str(), 1);
            setenv("AMBER_PASSWORD", amber->license.password.c_str(), 1);
            setenv("AMBER_SERVER", amber->license.server.c_str(), 1);
            setenv("AMBER_OATH_SERVER", amber->license.oauthserver.c_str(), 1);
            delete amber;
        } catch (amber_except &e) {
            assert("Exception loading AMBER_TEST_FILE");
        }
    } else {
        // load profile for testing through secrets manager
        char *profile = getenv("AMBER_TEST_PROFILE");
        if (!profile) {
            assert("AMBER_TEST_FILE or AMBER_TEST_PROFILE are required in the environment");
        }
        json secrets = get_secrets();
        if (!secrets.contains(profile)) {
            assert("invalid license identifier");
        }

        setenv("AMBER_USERNAME", std::string(secrets[profile]["username"]).c_str(), 1);
        setenv("AMBER_PASSWORD", std::string(secrets[profile]["password"]).c_str(), 1);
        setenv("AMBER_SERVER", std::string(secrets[profile]["server"]).c_str(), 1);
        setenv("AMBER_OAUTH_SERVER", secrets[profile].contains("oauth-server")
                                     ? std::string(secrets[profile]["oauth-server"]).c_str()
                                     : std::string(secrets[profile]["server"]).c_str(), 1);
    }
}

json clear_env_variables() {

    json saved_profile{
        {"AMBER_LICENSE_FILE", NULL},
        {"AMBER_LICENSE_ID",   NULL},
        {"AMBER_USERNAME",     NULL},
        {"AMBER_PASSWORD",     NULL},
        {"AMBER_SERVER",       NULL},
        {"AMBER_OAUTH_SERVER", NULL},
        {"AMBER_SSL_CERT",     NULL},
        {"AMBER_SSL_VERIFY",   NULL}
    };

    for (auto it: saved_profile.items()) {
        if (getenv(it.key().c_str())) {
            saved_profile[it.key()] = getenv(it.key().c_str());
            unsetenv(it.key().c_str());
        }
    }
    return saved_profile;
}

void restore_env_variables(json saved_env) {
    // clear out existing environment
    for (auto it: env_mappings.items()) {
        unsetenv(it.key().c_str());
    }

    // load environment from saved profile
    for (auto it: saved_env.items()) {
        if (it.value() != NULL) {
            setenv(it.key().c_str(), std::string(saved_env[it.key()]).c_str(), 1);
        }
    }
}