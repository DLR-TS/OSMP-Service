﻿/**
@authors German Aerospace Center: Björn Bahn
*/

#ifndef RECORD_H
#define RECORD_H

#include <fstream>

#include "Utils.h"
#include "ServiceInterface.h"

class Record : public ServiceInterface {
	virtual int create(const std::string& path) override;
	virtual void init(bool verbose, float starttime = 0) override;

	virtual int writeOSIMessage(const std::string& name, const std::string& value) override;
	virtual std::string readOSIMessage(const std::string& name) override;
	virtual int doStep(double stepSize) override;

private:
	std::string fileName;
	std::ofstream logFile;
	eOSIMessage outputMessageType;
};

#endif // !RECORD_H