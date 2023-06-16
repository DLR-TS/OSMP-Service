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
	virtual int readOSIMessage(const std::string& name, std::string& message) override;
	virtual int doStep(double stepSize) override;
	virtual void close() override;
private:
	std::ifstream filestream;
	std::vector<std::string> currentLine;
	unsigned long long timeOffsetMicros;
	unsigned long long simulationTimeMicros;

	std::vector<std::string> parseNextLine();
	int createTrafficUpdateMessage(osi3::TrafficUpdate& trafficUpdate);
	void createMovingObject(const std::vector<std::string>& values, osi3::MovingObject* movingObject);

	uint8_t indexTS;
	uint8_t indexID;
	uint8_t indexHeight;
	uint8_t indexWidth;
	uint8_t indexLength;
	uint8_t indexClass;
	uint8_t indexVelocityX;
	uint8_t indexVelocityY;
	uint8_t indexAccelerationX;
	uint8_t indexAccelerationY;
	uint8_t indexOrientation;
	uint8_t indexPositionX;
	uint8_t indexPositionY;
	uint8_t indexPositionZ;
};

#endif // !PLAYBACK_H
