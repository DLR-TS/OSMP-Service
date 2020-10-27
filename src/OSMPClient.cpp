#include "OSMPClient.h"
#include "OSMPInterface.h"
#include <filesystem>

int main(int argc, char *argv[])
{
	std::cout << "Welcome to OSMPClient." << std::endl << std::endl;

	std::cout << std::filesystem::current_path() << std::endl << std::endl;

	OSMPInterface FMUInterface;
	//FMUInterface.create("path");
	//FMUInterface.init();//float starttime = 0
	simulationLoop();

	return 0;
}

void simulationLoop() {
	//wait for gRPC calls
	//do stuff
	//return to gRPC
}