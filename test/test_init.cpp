#include "amber_sdk.h"
#include "secrets.h"
#include <gtest/gtest.h>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace {

TEST(amber_sdk, Environment) { ASSERT_TRUE(getenv("AMBER_TEST_LICENSE_ID")); }

TEST(amber_sdk, Constructors) {

  json saved_env = clear_env_variables();

  amber_sdk *amber = NULL;

  // set credentials using license file
  try {
    amber = new amber_sdk("default", "test/test.Amber.license");
    ASSERT_EQ(amber->license.username, "admin");
    ASSERT_EQ(amber->license.password, "admin");
    ASSERT_EQ(amber->license.server, "http://localhost:5007/v1");
    ASSERT_EQ(amber->license.oauth_server, "http://localhost:5007/v1");
    delete amber;
  } catch (amber_except &e) {
    ASSERT_TRUE(false) << e.what();
  }

  // set credentials using license file specified via environment variables
  try {
    setenv("AMBER_LICENSE_FILE", "test/test.Amber.license", 1);
    setenv("AMBER_LICENSE_ID", "default", 1);
    amber = new amber_sdk("", "");
    ASSERT_EQ(amber->license.username, "admin");
    ASSERT_EQ(amber->license.password, "admin");
    ASSERT_EQ(amber->license.server, "http://localhost:5007/v1");
    ASSERT_EQ(amber->license.oauth_server, "http://localhost:5007/v1");
    delete amber;
  } catch (amber_except &e) {
    ASSERT_TRUE(false) << e.what();
  }

  // set credentials using license file specified via environment variables
  try {
    unsetenv("AMBER_LICENSE_FILE");
    unsetenv("AMBER_LICENSE_ID");
    setenv("AMBER_USERNAME", "xyyyAmberUser", 1);
    setenv("AMBER_PASSWORD", "bogus_password", 1);
    setenv("AMBER_SERVER", "https://temp.amber.boonlogic.com/v1", 1);
    setenv("AMBER_SSL_CERT", "bogus_ssl_cert", 1);
    setenv("AMBER_SSL_VERIFY", "false", 1);
    amber = new amber_sdk("", "");
    EXPECT_EQ(amber->license.username, "xyyyAmberUser");
    EXPECT_EQ(amber->license.password, "bogus_password");
    EXPECT_EQ(amber->license.server, "https://temp.amber.boonlogic.com/v1");
    delete amber;
  } catch (amber_except &e) {
    ASSERT_TRUE(false) << e.what();
  }

  restore_env_variables(saved_env);
}

TEST(amber_sdk, ConstructorsNegative) {
  json saved_env = clear_env_variables();

  EXPECT_THROW(new amber_sdk("default", "nonexistent-license-file"),
               amber_except);
  EXPECT_THROW(
      new amber_sdk("nonexistent-license-id", "test/test.Amber.license"),
      amber_except);
  EXPECT_THROW(new amber_sdk("missing-username", "test/test.Amber.license"),
               amber_except);
  EXPECT_THROW(new amber_sdk("missing-password", "test/test.Amber.license"),
               amber_except);
  EXPECT_THROW(new amber_sdk("missing-server", "test/test.Amber.license"),
               amber_except);

  restore_env_variables(saved_env);
}
} // namespace