#include "GRPCInterface.h"
#include <filesystem>

int main(int argc, char *argv[])
{
	std::cout << "Welcome to OSMPClient." << std::endl << std::endl;

	std::cout << "Current directory: " << std::filesystem::current_path() << std::endl << std::endl;

	std::string server_address = "0.0.0.0:51425";
	if (1 < argc) {
		server_address = argv[1];
	}

	GRPCInterface grpc(server_address);
	
	grpc.startServer();
	
	return 0;
}
