#include "GRPCInterface.h"
#include <filesystem>

int main(int argc, char *argv[])
{
	std::cout << "Welcome to OSMPClient.\n";
	std::cout << "Current directory: " << std::filesystem::current_path() << "\n" << std::endl;

	//Server address deliberately chosen to accept any connection
	std::string server_address = "0.0.0.0:51425";
	bool verbose = false;

	for (int i = 1; i < argc; i++) {
		const std::string parameter = std::string(argv[i]);
		if (parameter == "-d" || parameter == "-v") {
			verbose = true;
			std::cout << "Verbose messages enabled." << std::endl;
		}
		else { //(ip &) port
			if (parameter.find(':') == std::string::npos)
			{
				//listen to messages from all ips
				server_address = "0.0.0.0:" + parameter;
			}
			else {
				server_address = parameter;
			}
		}
	}

	std::cout << "Server listens on: " << server_address << std::endl;

	GRPCInterface grpc(server_address, verbose);
	grpc.startServer();

	return 0;
}
