/**
@authors German Aerospace Center: Björn Bahn
*/

#ifndef PLAYBACK_H
#define PLAYBACK_H

#define _USE_MATH_DEFINES

#include <fstream>
#include <sstream>
#include <cmath>

#include "ServiceInterface.h"
#include "Utils.h"

class Playback : public ServiceInterface {
	virtual int create(const std::string& path) override;
	virtual void init(bool verbose, float starttime = 0) override;

	virtual int writeOSIMessage(const std::string& name, const std::string& value) override;
	virtual std::string readOSIMessage(const std::string& name) override;
	virtual int doStep(double stepSize) override;
	virtual void close() override;
private:
	std::ifstream filestream;
	std::vector<std::string> currentLine;
	unsigned long long timeOffsetMicros;
	unsigned long long simulationTimeMicros;

	std::vector<std::string> parseNextLine();
	osi3::TrafficUpdate createTrafficUpdateMessage();
	void createMovingObject(const std::vector<std::string>& values, osi3::MovingObject* movingObject);
};

#endif // !PLAYBACK_H
