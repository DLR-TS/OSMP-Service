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
	virtual int create(const std::string& path) override;
	virtual void init(bool verbose, OSMPTIMEUNIT timeunit, float starttime = 0) override;

	virtual int writeOSIMessage(const std::string& name, const std::string& value) override;
	virtual int readOSIMessage(const std::string& name, std::string& message) override;
	virtual int doStep(double stepSize) override;
	virtual void close() override;
private:
	std::map<std::string, std::ofstream*> output;
	std::thread writeThread;

	void saveImage(const osi3::SensorView sensorView, const std::string name);
	std::string formatTimeToMS(const osi3::Timestamp& timestamp);
};

#endif // !RECORD_H
