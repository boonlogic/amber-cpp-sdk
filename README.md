# Boon Amber C++ SDK

An SDK for Boon Amber sensor analytics

- __Website__: [boonlogic.com](https://boonlogic.com)
- __Documentation__: [Boon Docs Main Page](https://docs.boonlogic.com)
- __SDK Functional Breakdown__: []()

## Installation

The Boon Amber C++ SDK is a source-only distribution.

amber-cpp-sdk uses libcurl to send and receive http requests.  This must be installed on the host system before building amber-cpp-sdk.

A full detail of curl installation options can be found at:
https://curl.haxx.se/download.html

Clone source:
```
git clone git@github.com/boonlogic/amber-cpp-sdk.git
```

Within amber-cpp-sdk repository, create makefiles:
```
cmake .
```

Make libraries and supporting example scripts:
```
make
```


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

The _~/.Amber.license_ file will be consulted by the Amber SDK to find and authenticate your account credentials with the Amber server. Credentials may optionally be provided instead via the environment variables `AMBER_USERNAME` and `AMBER_PASSWORD`.

## Connectivity test

The following example provides a basic proof-of-connectivity:

[connect-example.cpp](examples/connect-example.cpp)

```c++
```

Running the connect-example.cpp example should yield output like the following:
```
$ cmake connect_example
sensors: {}
```
where the dictionary `{}` lists all sensors that currently exist under the given Boon Amber account.

## Full Example

The following provides and example of each amber-cpp-sdk method.

[full-example.cpp](examples/full-example.py)

```c++
```


## Advanced CSV file processor

The following will process a CSV file using batch-style streaming requests.  Full Amber analytic results will be displayed after each streaming request.  

[stream-advanced.cpp](examples/stream_advanced.cpp)<br>
[output_current.csv](examples/output_current.csv)

```c++
```

### Sample output:

```
State: Monitoring(0%), inferences: 20201, clusters: 247, samples: 20275, duration: 228.852
ID: 29,30,31,32,33,34,35,36,37,38,39,245,41,42,219,44,45,220,47,48,49,50,51,52,1 
SI: 306,307,307,307,307,307,307,308,308,308,308,308,309,311,315,322,336,364,421,532,350,393,478,345,382 
AD: 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 
AH: 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 
AM: 0.000013,0.000013,0.000013,0.000013,0.000013,0.000013,0.000013,0.000013,0.000013,0.000013,0.000013,0.000013,0.000013,0.000013,0.000013,0.000013,0.000013,0.000013,0.000013,0.000013,0.000013,0.000013,0.000013,0.000013,0.000013 
AW: 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 
```