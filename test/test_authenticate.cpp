#include "amber_sdk.h"
#include "secrets.h"
#include <gtest/gtest.h>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#else

#endif

namespace {

TEST(authenticate, Successful) {
  try {
    auto *amber = create_amber_client();
    amber_models::list_sensors_response response;
    EXPECT_TRUE(amber->list_sensors(response));
    delete amber;
  } catch (amber_except &e) {
    ASSERT_TRUE(false) << e.what();
  }
}

TEST(authenticate, Negative) {
  try {
    auto *amber = create_amber_client();
    amber->license.password = "bad-password";
    amber_models::list_sensors_response response;
    EXPECT_FALSE(amber->list_sensors(response));
    delete amber;
  } catch (amber_except &e) {
    ASSERT_TRUE(false) << e.what();
  }
}
} // namespace
