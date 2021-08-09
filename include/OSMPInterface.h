/**
@authors German Aerospace Center: Nils Wendorff, Björn Bahn, Danny Behnecke
*/

#ifndef OSMPInterface_H
#define OSMPInterface_H
#define NOMINMAX

#include <string>
#include <map>
#include <thread>

#include "fmi4cpp/fmi4cpp.hpp"

#include "OSIMessages.h"

class OSMPInterface 
{
public:
	int create(const std::string& path);
	int init(bool debug, float starttime = 0);
	std::string read(const std::string& name);
	int doStep(double stepSize = 1);
	void setParameter(std::vector<std::pair<std::string, std::string>>&);
	int write(const std::string& name,const std::string& value);
	int close();

	enum FMUState {
		UNINITIALIZED = 0,
		IN_INITIALIZATION_MODE = 1,//can perform algebraic loops for initialization
		INITIALIZED=2,//ready for use, can perform steps etc.
		TERMINATED=8// fmu was shutdown/destroyed/whatever
	};

protected:
	class OSMPFMUSlaveStateWrapper {
	private:
		//cs_slave creating this fmu state is needed later for freeing the memory again
		std::shared_ptr<fmi4cpp::fmi2::cs_slave> coSimSlave;

		OSMPFMUSlaveStateWrapper(std::shared_ptr < fmi4cpp::fmi2::cs_slave> slave);

	public:
		~OSMPFMUSlaveStateWrapper();

		fmi4cpp::fmi4cppFMUstate state;
		static std::optional<OSMPFMUSlaveStateWrapper> tryGetStateOf(std::shared_ptr<fmi4cpp::fmi2::cs_slave> slave);
	};

private:
	//fmi4cpp::fmi4cppFMUstate state;
	std::unique_ptr<fmi4cpp::fmi2::cs_fmu> coSimFMU;
	std::shared_ptr<fmi4cpp::fmi2::cs_slave> coSimSlave;
	FMUState fmuState = UNINITIALIZED;

	/**
	Temporary storage for osmp messages (name, size, address)
	*/
	std::map<std::string, address> fromFMUAddresses, toFMUAddresses;
	/**
	Save the annotated value in the address map. Supported names are count, valid, <>.base.hi , <>.base.lo, <>.size.
	\param std::map<std::string, address> &addressMap The map, the value is mapped in.
	\param std::string name name of the variable. Supported names are count, valid, <>.base.hi , <>.base.lo, <>.size.
	\param int value The value to be stored.
	*/
	void saveToAddressMap(std::map<std::string, address> &addressMap, const std::string& name, int value);

	int readOutputPointerFromFMU();
	int writeInputPointerToFMU();
	std::string readFromHeap(const address& address);
	int writeToHeap(address& address, const std::string& value);

	eOSIMessage getMessageType(const std::string& messageType);

	osi3::SensorView sensorView;
	osi3::SensorViewConfiguration sensorViewConfiguration;
	osi3::SensorData sensorData;
	osi3::GroundTruth groundTruth;
	osi3::TrafficCommand trafficCommand;
	osi3::TrafficUpdate trafficUpdate;
	setlevel4to5::MotionCommand_Trajectory trajectory;
	setlevel4to5::MotionCommand motionCommand;
	setlevel4to5::VehicleCommunicationData vehicleCommunicationData;

	/**
	stores the field \"valid\" from fmi
	*/
	bool valid = true;
	/**
	stores the field \"count\" from fmi
	*/
	int count;

	bool debug = false;
};

#endif // !OSMPInterface_H
