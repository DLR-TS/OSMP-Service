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
	void init(bool verbose, float starttime = 0);
	void setInitialParameter(const std::string& name, const std::string& value);
	void finishInitialization();

	int writeOSIMessage(const std::string& name,const std::string& value);
	std::string readOSIMessage(const std::string& name);

	int doStep(double stepSize = 1);

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
	std::shared_ptr<const fmi4cpp::fmi2::cs_model_description> modelDescription;

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
	void writeInputPointerToFMU();
	bool matchingNames(const std::string& name1, const std::string& name2);
	std::string readFromHeap(const address& address);
	void writeToHeap(address& address, const std::string& value);

	eOSIMessage getMessageType(const std::string& messageType);

	osi3::SensorView sensorView;
	osi3::SensorViewConfiguration sensorViewConfiguration;
	osi3::SensorData sensorData;
	osi3::GroundTruth groundTruth;
	osi3::TrafficCommand trafficCommand;
	osi3::TrafficUpdate trafficUpdate;

	/**
	stores the field \"valid\" from fmi
	*/
	bool valid = true;
	/**
	stores the field \"count\" from fmi
	*/
	//int count;
	/**
	* verbose logs
	*/
	bool verbose = false;
};

#endif // !OSMPInterface_H
