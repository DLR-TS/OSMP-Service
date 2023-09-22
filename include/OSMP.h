/**
@authors German Aerospace Center: Nils Wendorff, Björn Bahn, Danny Behnecke
*/

#ifndef OSMP_H
#define OSMP_H
#define NOMINMAX

#include <string>
#include <map>
#include <thread>

#include "fmi4cpp/fmi4cpp.hpp"
#include "ServiceInterface.h"
#include "Utils.h"
#include "OSIMessages.h"

class OSMP : public ServiceInterface
{
public:
	virtual int create(const std::string& path) override;
	virtual void init(bool verbose, bool nano, float starttime = 0) override;
	virtual void finishInitialization() override;

	virtual int writeOSIMessage(const std::string& name, const std::string& value) override;
	virtual int readOSIMessage(const std::string& name, std::string& message) override;
	virtual int doStep(double stepSize) override;
	virtual void close() override;
	void setInitialParameter(const std::string& name, const std::string& value) override;
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
	\param fmi2ValueReference valueReference
	\param std::string name name of the variable. Supported names are count, valid, <>.base.hi , <>.base.lo, <>.size.
	\param int value The value to be stored.
	*/
	void saveToAddressMap(std::map<std::string, address> &addressMap, const fmi2ValueReference valueReference, const std::string& name, int value);

	int readOutputPointerFromFMU();
	void writeInputPointerToFMU();
	std::string readFromHeap(const address& address);
	void writeToHeap(address& address, const std::string& value);

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
};

#endif // !OSMP_H
