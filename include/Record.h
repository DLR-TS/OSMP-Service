/**
@authors German Aerospace Center: Björn Bahn
*/

#ifndef RECORD_H
#define RECORD_H

#include <fstream>
#include <map>
#include <thread>
#include <boost/gil.hpp>
#include <boost/gil/io/write_view.hpp>
#include <boost/gil/extension/io/png.hpp>

#include "Utils.h"
#include "ServiceInterface.h"

class Record : public ServiceInterface {
public:
	virtual int create(const std::string& path) override;
	virtual void init(bool verbose, OSMPTIMEUNIT timeunit, float starttime = 0) override;

	virtual int writeOSIMessage(const std::string& name, const std::string& message) override;
	virtual int writeParameter(const std::string& name, const std::string& value) override;

	virtual int readOSIMessage(const std::string& name, std::string& message) override;
	virtual int readParameter(const std::string& name, std::string& value) override;

	virtual int doStep(double stepSize) override;
	virtual void close() override;

	void saveImage(const std::string& name, const osi3::SensorView& sensorView);
private:
	struct RecordFileName {
		std::string firstPart;
		uint32_t amount;
		std::string secondPart;
	};

	std::thread writeThread;
	std::string protobufVersion = std::to_string(GOOGLE_PROTOBUF_VERSION);
	std::map<std::string, RecordFileName> output;

	std::string formatTimeToMS(const osi3::Timestamp& timestamp);
	RecordFileName createCompliantNameForOSITraceFile(const std::string& message, const std::string& name, const eOSIMessage& messageType);
	std::string getISO8601Timestamp(int64_t& seconds);
	std::string getVersion(osi3::InterfaceVersion& version);
	std::string asString(RecordFileName& fileName) {
		return fileName.firstPart + std::to_string(fileName.amount) + fileName.secondPart;
	}
};

#endif // !RECORD_H
