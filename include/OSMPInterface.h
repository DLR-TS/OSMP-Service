#ifndef OSMPInterface_H
#define OSMPInterface_H
#define NOMINMAX
#include <string>
#include "fmi4cpp/fmi4cpp.hpp"

class OSMPInterface 
{
public:
	int create(std::string path);
	int init(float starttime = 0);
	int read();
	int write();
	int close();
private:
	//fmi4cpp::fmi4cppFMUstate state;
	std::unique_ptr<fmi4cpp::fmi2::cs_fmu> coSimFMU;
	std::shared_ptr<fmi4cpp::fmi2::cs_slave> coSimSlave;

	/**
stores the field \"valid\" from fmi
*/
	bool valid = true;
	/**
	stores the field \"count\" from fmi
	*/
	int count;
};

#endif // !OSMPInterface_H