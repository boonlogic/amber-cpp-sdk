#include "amber_sdk.h"
#include <iostream>

int main(int argc, char *argv[]) {

  bool verify = true;
  char *cert = nullptr;
  char *cainfo = nullptr;

  for (int arg = 1; arg < argc; arg++) {
    std::string str(argv[arg]);
    while (str.find('-') == 0) {
      str.erase(0, 1);
    }

#define OPT_VERIFY "noverify"
#define OPT_CERT "cert="
#define OPT_CAPATH "cainfo="

    if (strcasecmp(OPT_VERIFY, str.c_str()) == 0) {
      verify = false;
    } else if (strncasecmp(OPT_CERT, str.c_str(), strlen(OPT_CERT)) == 0) {
      char *s = (char *)str.c_str() + strlen(OPT_CERT);
      cert = strdup(s);
    } else if (strncasecmp(OPT_CAPATH, str.c_str(), strlen(OPT_CAPATH)) == 0) {
      char *s = (char *)str.c_str() + strlen(OPT_CAPATH);
      cainfo = strdup(s);
    } else {
      if (strcasecmp("help", str.c_str()) != 0) {
        std::cout << "error: unknown argument '" << str << "'\n";
      }
      std::cout << "usage: " << argv[0] << " [--" << OPT_VERIFY << "] [--"
                << OPT_CERT << "<certificate>] [--" << OPT_CAPATH
                << "<cainfo>]\n";
      exit(1);
    }
  }

  // set up handler
  amber_sdk *amber;
  try {
    amber = new amber_sdk();
    if (!verify) {
      amber->verify_certificate(verify);
    }
    if (cainfo != nullptr) {
      amber->set_cainfo(cainfo);
    }
    if (cert != nullptr) {
      amber->set_cert(cert);
    }
  } catch (amber_except &e) {
    std::cout << e.what() << "\n";
    exit(1);
  }

  // get the version
  get_version_response version_response;
  auto err = amber->get_version(version_response);
  if (!err) {
    version_response.dump();
  } else {
    err->dump();
  }
}
