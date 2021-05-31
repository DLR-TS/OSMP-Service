#include "GRPCInterface.h"
#include <filesystem>

int main(int argc, char *argv[])
{
	bool debug = false;

	std::cout << "Welcome to OSMPClient." << std::endl;

	std::cout << "Current directory: " << std::filesystem::current_path() << std::endl << std::endl;

	std::string server_address = "0.0.0.0:51425";
	if (1 < argc) {
		server_address = argv[1];
		if (2 < argc) {
			std::string argument(argv[2]);
			if (argument == "-d") {
				debug = true;
				std::cout << "Running in debug mode." << std::endl << std::endl;
			}
		}
	}

	GRPCInterface grpc(server_address, debug);
	
	grpc.startServer();
	
	return 0;
}
