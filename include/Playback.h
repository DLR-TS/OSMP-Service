/**
@authors German Aerospace Center: Nils Wendorff, Björn Bahn, Danny Behnecke
*/

#ifndef PLAYBACK_H
#define PLAYBACK_H

#include <fstream>
#include <sstream>
#include <queue>
#include <cmath>

#include "ServiceInterface.h"
#include "Utils.h"

class Playback : public ServiceInterface {
	virtual int create(const std::string& path) override;
	virtual void init(bool verbose, float starttime = 0) override;

	virtual int writeOSIMessage(const std::string& name, const std::string& value) override;
	virtual std::string readOSIMessage(const std::string& name) override;
	virtual int doStep(double stepSize) override;
private:
	std::queue<std::vector<std::string>> parsedCsv;
	unsigned long long timeOffsetMicroSeconds;
	double simulationTimeSeconds;

	osi3::TrafficUpdate createTrafficUpdateMessage();
	void createMovingObject(const std::vector<std::string>& values, osi3::MovingObject* movingObject);
};

#endif // !PLAYBACK_H
