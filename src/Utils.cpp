#include "Utils.h"

eOSIMessage getMessageType(const std::string& messageType) {
	if (messageType.find("SensorView") != std::string::npos
		&& messageType.find("Config") == std::string::npos) {
		return SensorViewMessage;
	}
	else if (messageType.find("SensorView") != std::string::npos
		&& messageType.find("Config") != std::string::npos) {
		return SensorViewConfigurationMessage;
	}
	else if (messageType.find("SensorData") != std::string::npos) { return SensorDataMessage; }
	else if (messageType.find("GroundTruth") != std::string::npos) { return GroundTruthMessage; }
	else if (messageType.find("TrafficCommand") != std::string::npos) { return TrafficCommandMessage; }
	else if (messageType.find("TrafficUpdate") != std::string::npos) { return TrafficUpdateMessage; }
	else {
		std::cout << "Error: Can not find message " << messageType << std::endl;
		throw - 1;
	}
}

bool matchingNames(const std::string& name1, const std::string& name2) {
	std::size_t openbracketposname1 = name1.find("[");
	//no index:
	if (openbracketposname1 == std::string::npos) {
		if (name1 == name2 || name1 == name2 + "In" || name1 == name2 + "Out"
			|| name2 == name1 + "In" || name2 == name1 + "Out")
			return true;
	}
	else {
		//with index
		std::string_view name1WithoutIndex = name1.substr(0, openbracketposname1);
		std::size_t openbracketposname2 = name2.find("[");
		std::string_view name2WithoutIndex = name2.substr(0, openbracketposname1);

		if (name2WithoutIndex.find(name1WithoutIndex) != std::string::npos
			|| name1WithoutIndex.find(name2WithoutIndex) != std::string::npos) {
			//matching names
			std::size_t closebracketposname1 = name1.find("]");
			std::size_t closebracketposname2 = name2.find("]");
			std::string_view name1_index = name1.substr(openbracketposname1 + 1, closebracketposname1 - 1);
			std::string_view name2_index = name2.substr(openbracketposname2 + 1, closebracketposname2 - 1);
			if (name1_index == name2_index) {
				//matching index
				return true;
			}
		}
	}
	return false;
}
