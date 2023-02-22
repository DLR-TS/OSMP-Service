#include "Record.h"

int Record::create(const std::string& name) {
	fileName = name;
	return 0;
}

void Record::init(bool verbose, float starttime) {
}

int Record::writeOSIMessage(const std::string& name, const std::string& value) {
	eOSIMessage messageType = getMessageType(name);
	if (messageType == eOSIMessage::SensorViewConfigurationMessage) {
		return 0;
	}
	if (!logFile.is_open()) {
		logFile.open(fileName);
		outputMessageType = messageType;
	}
	if (messageType == outputMessageType) {
		//format: size as long, message
		logFile << (long)value.size();
		logFile << value;
		logFile << std::flush;
	}
	else {
		std::cerr << "Different message types in one file are not supported by OSMP-Service. Please reconfigure your simulation setup." << std::endl;
	}
	return 0;
}

std::string Record::readOSIMessage(const std::string& name) {
	std::cout << "Nothing to read from " << name << std::endl;
	if (getMessageType(name) == eOSIMessage::SensorViewConfigurationMessage) {
		osi3::SensorViewConfiguration c;
		return c.SerializeAsString();
	}
	return "";
}

int Record::doStep(double stepSize) {
	return 0;
}
