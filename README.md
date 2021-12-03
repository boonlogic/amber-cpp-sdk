# Boon Amber C++ SDK

An SDK for Boon Amber sensor analytics

- __Website__: [boonlogic.com](https://boonlogic.com)
- __Documentation__: [Boon Docs Main Page](https://docs.boonlogic.com)
- __SDK Functional Breakdown__: []()

## Installation

The Boon Amber C++ SDK is a source-only distribution.

amber-cpp-sdk uses libcurl to send and receive http requests.  This must be installed on the host system before building amber-cpp-sdk.

A full detail of curl installation options can be found at:
[https://curl.haxx.se/download.html](https://curl.haxx.se/download.html])

Clone source:

```bash
git clone git@github.com/boonlogic/amber-cpp-sdk.git
```

Within amber-cpp-sdk repository, create makefiles:

```bash
cmake .
```

Make libraries and supporting example scripts:

```bash
make
```

[Internal Developers Notes](README-dev.md)

## Credentials setup

Note: An account in the Boon Amber cloud must be obtained from Boon Logic to use the Amber SDK.

The username and password should be placed in a file named _~/.Amber.license_ whose contents are the following:

```json
{
    "default": {
        "username": "AMBER-ACCOUNT-USERNAME",
        "password": "AMBER-ACCOUNT-PASSWORD",
        "server": "https://amber.boonlogic.com/v1"
    }
}
```

The ~/.Amber.license file will be consulted by the Amber SDK to find and authenticate your account credentials with the Amber server. Credentials may optionally be provided instead via the environment variables `AMBER_USERNAME` and `AMBER_PASSWORD`.

## Connectivity test

The following example provides a basic proof-of-connectivity:

[connect_example.cpp](examples/connect_example.cpp)

Running the bin/connect-example should yield output like the following:
```
$ bin/connect-example
{
    "api-version": "/v1",
    "builder": "712de01c",
    "expert-api": "55bb36dd",
    "expert-common": "f65c90bf",
    "nano-secure": "aac9f5d0",
    "release": "dev",
    "swagger-ui": "914af396"
}

```

## Examples

[full-example.cpp](examples/full_example.cpp) : demonstrates each sdk call (bin/full-example)

[stream-advanced.cpp](examples/stream_advanced.cpp) : streams output_current.csv to Amber and displays analytic results (bin/full-example)

[output_current.csv](examples/output_current.csv) : supporting csv file
