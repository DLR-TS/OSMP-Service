#include "GRPCInterface.h"
#include <filesystem>

int main(int argc, char *argv[])
{
	bool debug = false;

	std::cout << "Welcome to OSMPClient." << std::endl;

	std::cout << "Current directory: " << std::filesystem::current_path() << "\n" << std::endl;

	//Server address deliberately chosen to accept any connection
	std::string server_address = "0.0.0.0:51425";

	for (int i = 1; i < argc; i++) {
		if (std::string(argv[i]) == "-d") {
			debug = true;
			std::cout << "Debug messages enabled." << std::endl;
		}
		else {
			server_address = argv[i];
		}
	}

	std::cout << "Server listens on: " << server_address << std::endl;

	GRPCInterface grpc(server_address, debug);
	
	grpc.startServer();
	
	return 0;
}
