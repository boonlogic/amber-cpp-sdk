.PHONY: generate

export LDFLAGS=-L/usr/local/opt/curl/lib -lcurl
export CPPFLAGS=-I/usr/local/opt/curl/include

depends:
	wget https://github.com/nlohmann/json/archive/v3.9.1.tar.gz -O - | tar -xz

#git clone --single-branch --branch v3.9.1 git@github.com:nlohmann/json.git json

build:
	g++ -g $(CPPFLAGS) -c sdk.cpp
	g++ -g -o try $(CPPFLAGS) try.cpp sdk.o $(LDFLAGS)
