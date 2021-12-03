# amber-cpp-sdk

## Developers notes

### clone package and construct cmake project
```
git clone git@gitlab.boonlogic.com:development/expert/amber-cpp-sdk
cd amber-cpp-sdk
cmake .
make
```

### running examples scripts
All project binaries (examples and test) can be found the bin directory

```
bin/connect-example 

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

### running the test_client

test_client is built and installed in the projects bin directory.  test_client requires
the environment variable `AMBER_TEST_PROFILE` to be set in order to select the correct
test platform.

The aws cli must be installed on the local test machine and the configured user must have an access
role for AWS secretsmanager.  See an administrator if you needs this.

The `runtest` command will assist in running the test client.

To run against the dev environment

```
./runtest dev
```

Running with no arguments will provide the full list of test environments
```
./runtest
error: enviroment is missing

runtest [v1,v1next,aop,aoc,dev,qa]
```

### publishing a new version of amber-cpp-sdk
TBD

### model generation
amber-cpp-sdk does not use automatic code generation when schema changes are made.  
