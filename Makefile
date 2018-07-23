CPP_IN=smart_account_creator

build:
	eosiocpp -o $(CPP_IN).wast $(CPP_IN).cpp

abi:
	eosiocpp -g $(CPP_IN).abi $(CPP_IN).cpp

all: build abi

clean:
	rm -f $(CPP_IN).wast $(CPP_IN).wasm
