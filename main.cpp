#include "sdk.h"

int main(int argc, char *argv[]) {

    amber_sdk *sdk = new amber_sdk();
    sdk->list_sensors();
}

