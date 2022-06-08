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
    list_sensors_response response;
    EXPECT_EQ(amber->list_sensors(response), nullptr);
    delete amber;
  } catch (amber_except &e) {
    ASSERT_TRUE(false) << e.what();
  }
}

TEST(authenticate, Negative) {
  try {
    auto *amber = create_amber_client();
    amber->license.password = "bad-password";
    list_sensors_response response;
    EXPECT_NE(amber->list_sensors(response), nullptr);
    delete amber;
  } catch (amber_except &e) {
    ASSERT_TRUE(false) << e.what();
  }
}
} // namespace
