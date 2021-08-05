#include "OSMPInterface.h"

int OSMPInterface::create(const std::string& path) {
	auto abs = std::filesystem::absolute(path);
	if (!std::filesystem::exists(abs)) {
		std::cout << "File does not exist: " << abs.string() << std::endl;
	}
	std::unique_ptr<fmi4cpp::fmi2::fmu> fmu = std::make_unique<fmi4cpp::fmi2::fmu>(abs.string());
	if (!fmu->supports_cs()) {
		// FMU contains no cs model
		return 216373;
	}

	// load co-simulation description from FMU
	coSimFMU = fmu->as_cs_fmu();
	auto modelDescription = coSimFMU->get_model_description();
	return 0;
}

int OSMPInterface::init(bool debug, float starttime) {
	//Instance name cannot be set with FMU4cpp. The model identifier is used automatically instead
	coSimSlave = coSimFMU->new_instance();
	if (debug)
		coSimSlave->set_debug_logging(true, { "OSI", "FMU", "OSMP" });

	coSimSlave->setup_experiment((fmi2Real)starttime);
	coSimSlave->enter_initialization_mode();
	fmuState = IN_INITIALIZATION_MODE;

	auto const model_description = coSimFMU->get_model_description();
	//iterate over unknowns declared as fmu inputs or outputs and create AddressMap
	for (auto const& var : *(model_description->model_variables)) {
		if (var.is_integer()) {
			// possible inputs of develop following version sl45/v1.0.1:
			// OSMPSensorViewIn, OSMPSensorDataIn, OSMPTrafficCommandIn of causality "input"
			// OSMPSensorViewInConfig, OSMPGroundTruthInit of causality "parameter"
			if (var.causality == fmi4cpp::fmi2::causality::input || fmi4cpp::fmi2::causality::parameter == var.causality) {
				fmi2Integer integer;
				coSimSlave->read_integer(var.value_reference, integer);
				saveToAddressMap(toFMUAddresses, var.name, integer);
			}
			else if (fmi4cpp::fmi2::causality::output == var.causality || fmi4cpp::fmi2::causality::calculatedParameter == var.causality) {
				fmi2Integer integer;
				coSimSlave->read_integer(var.value_reference, integer);
				saveToAddressMap(fromFMUAddresses, var.name, integer);
			}
		}
	}
	return 0;
}

void OSMPInterface::setParameter(std::vector<std::pair<std::string, std::string>>& parameters) {

	if (parameters.size() == 0) {
		return;
	}

	auto const model_description = coSimSlave->get_model_description();
	//iterate over unknowns declared as fmu inputs or outputs and create AddressMap
	for (auto const& var : *(model_description->model_variables)) {
		if (var.is_string() & var.causality == fmi4cpp::fmi2::causality::parameter) {
			
			for (auto& parameter : parameters) {
				if (parameter.first == var.name) {
					coSimSlave->write_string(var.value_reference, parameter.second.c_str());
					std::cout << "Set Parameter: " << parameter.first << " Value: " << parameter.second << "\n";
				}
			}
		}
	}
	std::cout << std::endl;//flush after all parameters
}

std::string OSMPInterface::read(const std::string& name) {
	if (fromFMUAddresses.size() == 0) {
		return "";
	}
	if (IN_INITIALIZATION_MODE == fmuState) {
		//update pointers
		readOutputPointerFromFMU();
	}
	//read message from FMU
	for (auto& address : fromFMUAddresses) {
		if (address.first == name) {
			return readFromHeap(address.second);
		}
	}
	return "";
}

int OSMPInterface::write(const std::string& name, const std::string& value) {
	if (toFMUAddresses.size() == 0) {
		return -1;
	}
	//write message to FMU 
	for (auto& address : toFMUAddresses) {
		if (address.first == name) {
			int result = writeToHeap(address.second, value);
			if (IN_INITIALIZATION_MODE == fmuState) {
				writeInputPointerToFMU();
			}
			return result;
		}
	}
	return -1;
}

std::string OSMPInterface::readFromHeap(const address& address) {
	switch (getMessageType(address.name)) {
	case SensorViewMessage:
		if (!sensorView.ParseFromArray((const void*)address.addr.address, address.size)) {
			return "";
		}
		return sensorView.SerializeAsString();
		break;
	case SensorViewConfigurationMessage:
		if (!sensorViewConfiguration.ParseFromArray((const void*)address.addr.address, address.size)) {
			return "";
		}
		return sensorViewConfiguration.SerializeAsString();
		break;
	case SensorDataMessage:
		if (!sensorData.ParseFromArray((const void*)address.addr.address, address.size)) {
			return "";
		}
		return sensorData.SerializeAsString();
		break;
	case GroundTruthMessage:
		if (!groundTruth.ParseFromArray((const void*)address.addr.address, address.size)) {
			return "";
		}
		return groundTruth.SerializeAsString();
		break;
	case TrafficCommandMessage:
		if (!trafficCommand.ParseFromArray((const void*)address.addr.address, address.size)) {
			return "";
		}
		return trafficCommand.SerializeAsString();
		break;
	case TrafficUpdateMessage:
		if (!trafficUpdate.ParseFromArray((const void*)address.addr.address, address.size)) {
			return "";
		}
		return trafficUpdate.SerializeAsString();
		break;
	case SL45TrajectoryMessage:
		if (!trajectory.ParseFromArray((const void*)address.addr.address, address.size)) {
			return "";
		}
		return trajectory.SerializeAsString();
		break;
	case SL45MotionCommandMessage:
		if (!motionCommand.ParseFromArray((const void*)address.addr.address, address.size)) {
			return "";
		}
		return motionCommand.SerializeAsString();
		break;
	case SL45VehicleCommunicationDataMessage:
		if (!vehicleCommunicationData.ParseFromArray((const void*)address.addr.address, address.size)) {
			return "";
		}
		return vehicleCommunicationData.SerializeAsString();
		break;
	}

};

int OSMPInterface::writeToHeap(address& address, const std::string& value) {
	if ((void*)address.addr.address != nullptr) {
		//free the allocated storage of previous osimessage
		free((void*)address.addr.address);
	}
	switch (getMessageType(address.name)) {
	case SensorViewMessage:
		sensorView.ParseFromString(value);
		address.size = (int)sensorView.ByteSizeLong();
		address.addr.address = (unsigned long long)malloc(address.size);
		sensorView.SerializeToArray((void*)address.addr.address, address.size);
		break;
	case SensorViewConfigurationMessage:
		sensorViewConfiguration.ParseFromString(value);
		address.size = (int)sensorViewConfiguration.ByteSizeLong();
		address.addr.address = (unsigned long long)malloc(address.size);
		sensorViewConfiguration.SerializeToArray((void*)address.addr.address, address.size);
		break;
	case SensorDataMessage:
		sensorData.ParseFromString(value);
		address.size = (int)sensorData.ByteSizeLong();
		address.addr.address = (unsigned long long)malloc(address.size);
		sensorData.SerializeToArray((void*)address.addr.address, address.size);
		break;
	case GroundTruthMessage:
		groundTruth.ParseFromString(value);
		address.size = (int)groundTruth.ByteSizeLong();
		address.addr.address = (unsigned long long)malloc(address.size);
		groundTruth.SerializeToArray((void*)address.addr.address, address.size);
		break;
	case TrafficCommandMessage:
		trafficCommand.ParseFromString(value);
		address.size = (int)trafficCommand.ByteSizeLong();
		address.addr.address = (unsigned long long)malloc(address.size);
		trafficCommand.SerializeToArray((void*)address.addr.address, address.size);
		break;
	case TrafficUpdateMessage:
		trafficUpdate.ParseFromString(value);
		address.size = (int)trafficUpdate.ByteSizeLong();
		address.addr.address = (unsigned long long)malloc(address.size);
		trafficUpdate.SerializeToArray((void*)address.addr.address, address.size);
		break;
	case SL45TrajectoryMessage:
		trajectory.ParseFromString(value);
		address.size = (int)trajectory.ByteSizeLong();
		address.addr.address = (unsigned long long)malloc(address.size);
		trajectory.SerializeToArray((void*)address.addr.address, address.size);
		break;
	case SL45MotionCommandMessage:
		motionCommand.ParseFromString(value);
		address.size = (int)motionCommand.ByteSizeLong();
		address.addr.address = (unsigned long long)malloc(address.size);
		motionCommand.SerializeToArray((void*)address.addr.address, address.size);
		break;
	case SL45VehicleCommunicationDataMessage:
		vehicleCommunicationData.ParseFromString(value);
		address.size = (int)vehicleCommunicationData.ByteSizeLong();
		address.addr.address = (unsigned long long)malloc(address.size);
		vehicleCommunicationData.SerializeToArray((void*)address.addr.address, address.size);
		break;
	}

	return 0;
};

int OSMPInterface::doStep(double stepSize) {
	if (IN_INITIALIZATION_MODE == fmuState) {
		coSimSlave->exit_initialization_mode();
		fmuState = INITIALIZED;
	}
	if (INITIALIZED != fmuState) {
		//cannot use an uninitialized fmu
		//TODO return type/value
		return (int)std::errc::operation_not_permitted;
	}
	writeInputPointerToFMU();
	//TODO which parts of FMIBridge::doStep are needed?
			//TODO set independent tunable parameters
		//TODO set continuous- and discrete-time inputs and optionally also the derivatives of the former

		//TODO support rollback in case step is incomplete?
	auto preStepState = OSMPFMUSlaveStateWrapper::tryGetStateOf(coSimSlave);

	//TODO step by stepSize
	if (!coSimSlave->step(stepSize)) {
		while (fmi4cpp::status::Pending == coSimSlave->last_status()) {
			//wait for asynchronous fmu to finish
			std::this_thread::sleep_for(std::chrono::microseconds(10));
		}

		switch (coSimSlave->last_status()) {
		case fmi4cpp::status::Fatal:
			return -1;//TODO decide on common error return values
		case fmi4cpp::status::Error:
			return -2;//TODO decide on common error return values
		case fmi4cpp::status::Discard:
			//If a pre step state could be captured and the slave supports step size variation, try performing smaller substeps instead of one stepSize step
			if (!preStepState || !coSimSlave->get_model_description()->can_handle_variable_communication_step_size) {
				return 2;
			}
			//restore state before failed step
			coSimSlave->set_fmu_state(preStepState.value().state);
			// perform some smaller substeps
			int substeps = 2;
			for (int i = 0; i < substeps; i++) {
				int err = doStep(stepSize / 2);
				if (0 != err) {
					return 2;
				}
			}
		}
	}

	//read all FMI fields to local memory
	readOutputPointerFromFMU();
	return 0;
}

int OSMPInterface::readOutputPointerFromFMU() {
	auto const model_description = coSimFMU->get_model_description();
	//iterate over unknowns declared as output
	for (auto const& unknown : model_description->model_structure->outputs) {
		// use index to translate unknown into scalar_variable. FMU ScalarVariable index begins at 1
		auto const& outputVar = (*model_description->model_variables.get())[unknown.index - 1];
		if (outputVar.is_integer()) {
			fmi2Integer integer;
			coSimSlave->read_integer(outputVar.value_reference, integer);
			saveToAddressMap(fromFMUAddresses, outputVar.name, integer);
		}
	}
	if (!valid) {
		std::cout << "OSMP config not valid" << std::endl;
		return 1;
	}
	return 0;
}

int OSMPInterface::writeInputPointerToFMU() {
	auto const model_description = coSimFMU->get_model_description();
	//set pointers of messages in fmi
	for (auto const& inputVar : *(model_description->model_variables)) {
		if (inputVar.causality == fmi4cpp::fmi2::causality::input && inputVar.is_integer()) {
			for (auto address : toFMUAddresses) {
				if (inputVar.name.find(address.first) != std::string::npos) {
					if (inputVar.name.find(".hi") != std::string::npos) {
						coSimSlave->write_integer(inputVar.value_reference, address.second.addr.base.hi);
					}
					else if (inputVar.name.find(".lo") != std::string::npos) {
						coSimSlave->write_integer(inputVar.value_reference, address.second.addr.base.lo);
					}
					else if (inputVar.name.find(".size") != std::string::npos) {
						coSimSlave->write_integer(inputVar.value_reference, address.second.size);
					}
				}
			}
		}
	}
	return 0;
}

int OSMPInterface::close() {
	return 0;
}


void OSMPInterface::saveToAddressMap(std::map<std::string, address> &addressMap, const std::string& name, int value) {
	//check for normal fmi variables count and valid
	if (name == "count") {
		this->count = value;
		return;
	}
	if (name == "valid") {
		this->valid = value;
		return;
	}

	if (0 == name.compare(name.length() - 8, 8, ".base.hi")) {
		std::string prefixWithIndex = name.substr(0, name.length() - 8);

		if (addressMap.find(prefixWithIndex) == addressMap.end()) {
			address a;
			a.addr.base.hi = value;
			a.name = prefixWithIndex;
			addressMap.insert({ prefixWithIndex , a });
		}
		else {
			addressMap.at(prefixWithIndex).addr.base.hi = value;
		}
	}
	else if (0 == name.compare(name.length() - 8, 8, ".base.lo")) {
		std::string prefixWithIndex = name.substr(0, name.length() - 8);

		if (addressMap.find(prefixWithIndex) == addressMap.end()) {
			address a;
			a.addr.base.lo = value;
			a.name = prefixWithIndex;
			addressMap.insert({ prefixWithIndex , a });
		}
		else {
			addressMap.at(prefixWithIndex).addr.base.lo = value;
		}
	}
	else if (0 == name.compare(name.length() - 5, 5, ".size")) {
		std::string prefixWithIndex = name.substr(0, name.length() - 5);

		if (addressMap.find(prefixWithIndex) == addressMap.end()) {
			address a;
			a.size = value;
			a.name = prefixWithIndex;
			addressMap.insert({ prefixWithIndex , a });
		}
		else {
			addressMap.at(prefixWithIndex).size = value;
		}
	}
}


inline OSMPInterface::OSMPFMUSlaveStateWrapper::OSMPFMUSlaveStateWrapper(std::shared_ptr<fmi4cpp::fmi2::cs_slave> slave) {
	slave->get_fmu_state(state);
	coSimSlave = slave;
}

inline OSMPInterface::OSMPFMUSlaveStateWrapper::~OSMPFMUSlaveStateWrapper() {
	coSimSlave->free_fmu_state(state);
}

std::optional<OSMPInterface::OSMPFMUSlaveStateWrapper> OSMPInterface::OSMPFMUSlaveStateWrapper::tryGetStateOf(std::shared_ptr<fmi4cpp::fmi2::cs_slave> slave) {
	if (slave->get_model_description()->can_get_and_set_fmu_state) {
		return OSMPInterface::OSMPFMUSlaveStateWrapper(slave);
	}
	return std::nullopt;
}

eOSIMessage OSMPInterface::getMessageType(const std::string& messageType) {
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
	else if (messageType.find("Trajectory") != std::string::npos) { return SL45TrajectoryMessage; }
	else if (messageType.find("MotionCommand") != std::string::npos) { return SL45MotionCommandMessage; }
	else if (messageType.find("VehicleCommunicationData") != std::string::npos) { return SL45VehicleCommunicationDataMessage; }
	else {
		std::cout << "Error: Can not find message " << messageType << std::endl;
		throw 5372;
	}
}
