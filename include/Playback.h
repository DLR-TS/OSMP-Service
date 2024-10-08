﻿/**
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
	virtual void init(bool verbose, OSMPTIMEUNIT timeunit, float starttime = 0) override;

	virtual int writeOSIMessage(const std::string& name, const std::string& message) override;
	virtual int writeParameter(const std::string& name, const std::string& value) override;

	virtual int readOSIMessage(const std::string& name, std::string& message) override;
	virtual int readParameter(const std::string& name, std::string& value) override;

	virtual int doStep(double stepSize) override;
	virtual void close() override;
private:

	std::ifstream filestream;
	std::vector<std::string> currentLine;
	unsigned long long timeOffsetMicros;
	long simulationTimeMicros;
	bool idIndexSet = false;
	/**
	 * CoSiMa does preSimulationStep(), DoStep() and postSimulationStep()
	 * DoStep increases the simulation timer in this playback class.
	 * To not skip the first entry in the playback file, the first DoStep() must not increase the simulation timer!!!
	 * If any changes to the timing system are made, check this behaviour!
	*/
	bool firstDoStep = true;

	/**
	 * Returns an empty vector if end of file is reached.
	*/
	std::vector<std::string> parseNextLine();
	int createTrafficUpdateMessage(osi3::TrafficUpdate& trafficUpdate);
	int createTrafficCommandMessage(osi3::TrafficCommand& trafficCommand);
	void createMovingObject(const std::vector<std::string>& values, osi3::MovingObject* movingObject);
	unsigned long long determineTimeOffset(OSMPTIMEUNIT& timeunit, std::string& timestamp);
	bool lineInTimestep();

	//TrafficUpdate
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
	uint8_t indexModelReference;
	//TrafficCommand
	uint8_t indexLongitudinalDistanceActionDistance;
	uint8_t indexSpeedActionAbsoluteTargetSpeed;

	bool trafficCommandComputed = false;
	std::string trafficCommandString;
};

#endif // !PLAYBACK_H
