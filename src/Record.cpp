#include "Record.h"

int Record::create(const std::string& name) {
	return 0;
}

void Record::init(bool verbose, float starttime) {
}

int Record::writeOSIMessage(const std::string& name, const std::string& value) {
	eOSIMessage messageType = getMessageType(name);
	if (messageType == eOSIMessage::SensorViewConfigurationMessage) {
		return 0;
	}

	auto file = output.find(name);
	if (file == output.end()) {
		std::ofstream* logFile = new std::ofstream(name + ".osi", std::ofstream::binary);
		output.emplace(name, logFile);
		file = output.find(name);
	}
	std::ofstream* stream = file->second;

	uint32_t size = value.size();
	//format: size as long, message
	stream->write((char*)&size, sizeof(size));
	*stream << value << std::flush;
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

void Record::close() {
	for (auto& file : output) {
		file.second->close();
		delete file.second;
		file.second = 0;
	}
	output.clear();
}
