#include "amber_sdk.h"
#include <gtest/gtest.h>
#include <iostream>
#include "secrets.h"

#ifdef _WIN32
#include <Windows.h>
#else

#endif

namespace {

    TEST(authenticate, Successful) {
        json saved_env = clear_env_variables();
        load_credentials_into_env();
        try {
            auto *amber = new amber_sdk(NULL, NULL);
            amber_models::list_sensors_response response;
            EXPECT_TRUE(amber->list_sensors(response));
            delete amber;
        } catch (amber_except &e) {
            ASSERT_TRUE(false) << e.what();
        }
        restore_env_variables(saved_env);
    }

    TEST(authenticate, Negative) {

        json saved_env = clear_env_variables();
        load_credentials_into_env();

        setenv("AMBER_PASSWORD", "not-valid", 1);
        try {
            auto *amber = new amber_sdk(NULL, NULL);
            amber_models::list_sensors_response response;
            EXPECT_FALSE(amber->list_sensors(response));
            unsetenv("AMBER_PASSWORD");
            delete amber;
        } catch (amber_except &e) {
            ASSERT_TRUE(false) << e.what();
        }
        restore_env_variables(saved_env);
    }
}
