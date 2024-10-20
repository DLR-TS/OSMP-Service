﻿/**
@authors German Aerospace Center: Björn Bahn
*/

#ifndef SERVICEINTERFACE_H
#define SERVICEINTERFACE_H

#if __has_include(<filesystem>)
	#include <filesystem>
	namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
	#include <experimental/filesystem>
	namespace fs = std::experimental::filesystem;
#else
	error "Missing the <filesystem> header."
#endif

#include <string>
#include "TimeUnits.h"
#include "OSIMessages.h"

class ServiceInterface
{
public:
	virtual int create(const std::string& path) = 0;
	virtual void init(bool verbose, OSMPTIMEUNIT timeunit, float starttime = 0) = 0;
	virtual void finishInitialization() {};

	virtual int writeOSIMessage(const std::string& name, const std::string& message) = 0;
	virtual int writeParameter(const std::string& name, const std::string& value) = 0;

	virtual int readOSIMessage(const std::string& name, std::string& message) = 0;
	virtual int readParameter(const std::string& name, std::string& value) = 0;

	virtual int doStep(double stepSize) = 0;
	virtual void close() = 0;

	virtual void setInitialParameter(const std::string& name, const std::string& value) {};

protected:
	bool verbose = false;
	OSMPTIMEUNIT timeunit = OSMPTIMEUNIT::MICRO;
};

#endif // !SERVICEINTERFACE_H
