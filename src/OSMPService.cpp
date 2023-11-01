#include "GRPCServer.h"

int main(int argc, char *argv[])
{
	std::cout << "Welcome to OSMPClient.\nCurrent directory: "
		<< std::filesystem::current_path() << "\n" << std::endl;

	//Server address deliberately chosen to accept any connection
	std::string server_address = "0.0.0.0:51425";
	bool verbose = false;
	int divider = 1;
	OSMPTIMEUNIT timeunit = OSMPTIMEUNIT::UNSPECIFIED;

	for (int i = 1; i < argc; i++) {
		const std::string parameter = std::string(argv[i]);
		if (parameter == "-v") {
			verbose = true;
			std::cout << "Verbose messages enabled." << std::endl;
		}
		else if (parameter == "-nano") {
			timeunit = OSMPTIMEUNIT::NANO;
		}
		else if (parameter == "-micro") {
			timeunit = OSMPTIMEUNIT::MICRO;
		}
		else if (parameter == "-milli") {
			timeunit = OSMPTIMEUNIT::MILLI;
		}
		else if (parameter == "-divide") {
			divider = std::stoi(std::string(argv[++i]));
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

	GRPCServer grpc(server_address, verbose, timeunit, divider);
	grpc.startServer();

	return 0;
}
