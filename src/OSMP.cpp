#include "OSMP.h"

int OSMP::create(const std::string& path) {
	auto abs = std::filesystem::absolute(path);
	if (!std::filesystem::exists(abs)) {
		std::cout << "File does not exist: " << abs.string() << std::endl;
	}
	std::unique_ptr<fmi4cpp::fmi2::fmu> fmu = std::make_unique<fmi4cpp::fmi2::fmu>(abs.string());
	if (!fmu->supports_cs()) {
		std::cout << "FMU contains no cs model" << std::endl;
		return -1;
	}

	// load co-simulation description from FMU
	coSimFMU = fmu->as_cs_fmu();
	if (verbose) {
		std::cout << "Try parsing model description" << std::endl;
	}
	modelDescription = coSimFMU->get_model_description();
	
	if (verbose) {
		std::cout << "Parsed model description successfully" << std::endl;
	}
	return 0;
}

void OSMP::init(bool verbose, float starttime) {
	this->verbose = verbose;
	//Instance name cannot be set with FMU4cpp. The model identifier is used automatically instead
	coSimSlave = coSimFMU->new_instance();
	if (verbose) {
		coSimSlave->set_debug_logging(true, { "OSI", "FMU", "OSMP" });
	}

	coSimSlave->setup_experiment((fmi2Real)starttime);
	coSimSlave->enter_initialization_mode();

	//iterate over unknowns declared as fmu inputs or outputs and create AddressMap
	for (auto const& var : *(modelDescription->model_variables)) {
		// OSMPSensorViewIn, OSMPSensorDataIn, OSMPTrafficCommandIn of causality "input"
		// OSMPSensorViewInConfig, OSMPGroundTruthInit of causality "parameter"
		if (var.is_integer() && var.causality == fmi4cpp::fmi2::causality::input || var.causality == fmi4cpp::fmi2::causality::parameter) {
			if (verbose) {
				std::cout << "In Ref: " << var.value_reference << " " << var.name << "\n";
			}
			saveToAddressMap(toFMUAddresses, var.name, 0);
		}
		else if (var.is_integer() && var.causality == fmi4cpp::fmi2::causality::output || var.causality == fmi4cpp::fmi2::causality::calculatedParameter) {
			if (verbose) {
				std::cout << "Out Ref: " << var.value_reference << " " << var.name << "\n";
			}
			fmi2Integer integer;
			coSimSlave->read_integer(var.value_reference, integer);
			saveToAddressMap(fromFMUAddresses, var.name, integer);
		}				
	}
	if (toFMUAddresses.size() == 0) {
		std::cout << "Write: No messages to FMU defined.\n";
	}
	if (fromFMUAddresses.size() == 0) {
		std::cout << "Read: No messages from FMU defined.\n";
	}
	std::cout << std::flush;
}

void OSMP::setInitialParameter(const std::string& name, const std::string& value) {
	if (verbose) {
		std::cout << "Set intial parameter: " << name << " to " << value << std::endl;
	}
	for (auto const& var : *(modelDescription->model_variables)) {
		if (var.causality == fmi4cpp::fmi2::causality::parameter && var.name != name) {
			continue;
		}
		if (var.is_boolean()) {
			coSimSlave->write_boolean(var.value_reference, std::stoi(value));
			return;
		}
		else if (var.is_integer()) {
			coSimSlave->write_integer(var.value_reference, std::stoi(value));
			return;
		}
		else if (var.is_real()) {
			coSimSlave->write_real(var.value_reference, std::stod(value));
			return;
		}
		else if (var.is_string()){
			coSimSlave->write_string(var.value_reference, value.c_str());
			return;
		}
	}
	std::cout << "Error: Could not set intial parameter: " << name << " to " << value << "\n"
	<< "Possible model parameter variables are:\n";
	for (auto const& var : *(modelDescription->model_variables)) {
		if (var.causality == fmi4cpp::fmi2::causality::parameter) {
			std::cout << var.name << "\n";
		}
	}
	std::cout << std::flush;
}

void OSMP::finishInitialization() {
	if (verbose) {
		std::cout << "Try to exit initialization mode\n";
	}
	coSimSlave->exit_initialization_mode();
}

std::string OSMP::readOSIMessage(const std::string& name) {
	if (verbose) {
		std::cout << "Read " << name << std::endl;
	}
	//read message from FMU
	for (auto& address : fromFMUAddresses) {
		if (verbose) {
			std::cout << "Found FMU Address: " << address.first << "\n";
		}
		if (matchingNames(address.first, name)) {
			return readFromHeap(address.second);
		}
	}
	std::cout << "Could not find matching message: " << name << std::endl;
	if (getMessageType(name) == eOSIMessage::SensorViewConfigurationMessage) {
		osi3::SensorViewConfiguration c;
		return c.SerializeAsString();
	}
	return "";
}

int OSMP::writeOSIMessage(const std::string& name, const std::string& value) {
	if (verbose) {
		std::cout << "Write " << name << " Length: " << value.size() << std::endl;
	}
	//write message to FMU
	for (auto& address : toFMUAddresses) {
		if (matchingNames(name, address.first)) {
			writeToHeap(address.second, value);
			return 0;
		}
	}
	if (verbose) {
		std::cout << "Write was not successful" << std::endl;
	}
	return -1;
}

std::string OSMP::readFromHeap(const address& address) {
	if (verbose) {
		std::cout << address.name << ": lo: " << address.addr.base.lo << " hi: " << address.addr.base.hi << " size: " << address.size << "\n";
	}
	if (address.addr.address == 0) {
		if (!verbose) {
			std::cerr << address.name << ": lo: " << address.addr.base.lo << " hi: " << address.addr.base.hi << " size: " << address.size << "\n";
		}
		std::cerr << "Pointer are not set correctly! Keep running with empty message." << std::endl;
		return "";
	}
	switch (getMessageType(address.name)) {
	case SensorViewMessage:
		if (!sensorView.ParseFromArray((const void*)address.addr.address, address.size)) {
			std::cerr << "SensorView: Parse from pointer unsuccessful" << std::endl;
			return "";
		}
		if (verbose) {
			std::cout << sensorView.DebugString() << std::endl;
		}
		return sensorView.SerializeAsString();
		break;
	case SensorViewConfigurationMessage:
		if (!sensorViewConfiguration.ParseFromArray((const void*)address.addr.address, address.size)) {
			std::cerr << "SensorViewConfiguration: Parse from pointer unsuccessful" << std::endl;
			return "";
		}
		if (verbose) {
			std::cout << sensorViewConfiguration.DebugString() << std::endl;
		}
		return sensorViewConfiguration.SerializeAsString();
		break;
	case SensorDataMessage:
		if (!sensorData.ParseFromArray((const void*)address.addr.address, address.size)) {
			std::cerr << "SensorData: Parse from pointer unsuccessful" << std::endl;
			return "";
		}
		if (verbose) {
			std::cout << sensorData.DebugString() << std::endl;
		}
		return sensorData.SerializeAsString();
		break;
	case GroundTruthMessage:
		if (!groundTruth.ParseFromArray((const void*)address.addr.address, address.size)) {
			std::cerr << "GroundTruth: Parse from pointer unsuccessful" << std::endl;
			return "";
		}
		if (verbose) {
			std::cout << groundTruth.DebugString() << std::endl;
		}
		return groundTruth.SerializeAsString();
		break;
	case TrafficCommandMessage:
		if (!trafficCommand.ParseFromArray((const void*)address.addr.address, address.size)) {
			std::cerr << "TrafficCommand: Parse from pointer unsuccessful" << std::endl;
			return "";
		}
		if (verbose) {
			std::cout << trafficCommand.DebugString() << std::endl;
		}
		return trafficCommand.SerializeAsString();
		break;
	case TrafficUpdateMessage:
		if (!trafficUpdate.ParseFromArray((const void*)address.addr.address, address.size)) {
			std::cerr << "TrafficUpdate: Parse from pointer unsuccessful" << std::endl;
			return "";
		}
		if (verbose) {
			std::cout << trafficUpdate.DebugString() << std::endl;
		}
		return trafficUpdate.SerializeAsString();
		break;
	}
	std::cerr << "No match of wanted message: " << address.name << std::endl;
	return "";
}

void OSMP::writeToHeap(address& address, const std::string& value) {
	if ((void*)address.addr.address != nullptr) {
		//free the allocated storage of previous osimessage
		free((void*)address.addr.address);
	}
	switch (getMessageType(address.name)) {
	case SensorViewMessage:
		sensorView.ParseFromString(value);
		address.size = (int)sensorView.ByteSizeLong();
		address.addr.address = (unsigned long long)malloc(address.size);
		if (verbose && value.size()) {
				std::cout << sensorView.DebugString() << std::endl;
		}
		sensorView.SerializeToArray((void*)address.addr.address, address.size);
		break;
	case SensorViewConfigurationMessage:
		sensorViewConfiguration.ParseFromString(value);
		address.size = (int)sensorViewConfiguration.ByteSizeLong();
		address.addr.address = (unsigned long long)malloc(address.size);
		if (verbose && value.size()) {
			std::cout << sensorViewConfiguration.DebugString() << std::endl;
		}
		sensorViewConfiguration.SerializeToArray((void*)address.addr.address, address.size);
		break;
	case SensorDataMessage:
		sensorData.ParseFromString(value);
		address.size = (int)sensorData.ByteSizeLong();
		address.addr.address = (unsigned long long)malloc(address.size);
		if (verbose && value.size()) {
			std::cout << sensorData.DebugString() << std::endl;
		}
		sensorData.SerializeToArray((void*)address.addr.address, address.size);
		break;
	case GroundTruthMessage:
		groundTruth.ParseFromString(value);
		address.size = (int)groundTruth.ByteSizeLong();
		address.addr.address = (unsigned long long)malloc(address.size);
		if (verbose && value.size()) {
			std::cout << groundTruth.DebugString() << std::endl;
		}
		groundTruth.SerializeToArray((void*)address.addr.address, address.size);
		break;
	case TrafficCommandMessage:
		trafficCommand.ParseFromString(value);
		address.size = (int)trafficCommand.ByteSizeLong();
		address.addr.address = (unsigned long long)malloc(address.size);
		if (verbose && value.size()) {
			std::cout << trafficCommand.DebugString() << std::endl;
		}
		trafficCommand.SerializeToArray((void*)address.addr.address, address.size);
		break;
	case TrafficUpdateMessage:
		trafficUpdate.ParseFromString(value);
		address.size = (int)trafficUpdate.ByteSizeLong();
		address.addr.address = (unsigned long long)malloc(address.size);
		if (verbose && value.size()) {
			std::cout << trafficUpdate.DebugString() << std::endl;
		}
		trafficUpdate.SerializeToArray((void*)address.addr.address, address.size);
		break;
	}
}

int OSMP::doStep(double stepSize) {
	if (verbose) {
		std::cout << "Dostep with stepsize " << stepSize << "\n";
	}

	writeInputPointerToFMU();

	//Possible rollback if step can not be done
	auto preStepState = OSMPFMUSlaveStateWrapper::tryGetStateOf(coSimSlave);

	if (!coSimSlave->step(stepSize)) {
		if (verbose) {
			std::cout << "Call aysnchronous FMU\n";
		}
		while (fmi4cpp::status::Pending == coSimSlave->last_status()) {
			//wait for asynchronous fmu to finish
			std::this_thread::sleep_for(std::chrono::microseconds(10));
		}

		switch (coSimSlave->last_status()) {
		case fmi4cpp::status::Fatal:
			std::cerr << "Status: Fatal\n";
			return -1;
		case fmi4cpp::status::Error:
			std::cerr << "Status: Error\n";
			return -2;
		case fmi4cpp::status::Discard:
			std::cout << "Status: Discard\n";
			//If a pre step state could be captured and the slave supports step size variation, try performing smaller substeps instead of one stepSize step
			if (!preStepState || !coSimSlave->get_model_description()->can_handle_variable_communication_step_size) {
				return 2;
			}
			//restore state before failed step
			coSimSlave->set_fmu_state(preStepState.value().state);
			if (verbose) {
				std::cout << "Perform some smaller substeps\n";
			}
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
	std::cout << std::flush;
	return 0;
}

int OSMP::readOutputPointerFromFMU() {
	//iterate over unknowns declared as output
	for (const auto& unknown : modelDescription->model_structure->outputs) {
		// use index to translate unknown into scalar_variable. FMU ScalarVariable index begins at 1
		const auto& outputVar = (*modelDescription->model_variables.get())[unknown.index - 1];
		if (outputVar.is_integer()) {
			fmi2Integer integer;
			coSimSlave->read_integer(outputVar.value_reference, integer);
			saveToAddressMap(fromFMUAddresses, outputVar.name, integer);
		}
	}
	if (!valid) {
		std::cout << "OSMP config not valid" << std::endl;
		return -1;
	}
	return 0;
}

void OSMP::writeInputPointerToFMU() {
	//set pointers of messages in fmi
	for (const auto& inputVar : *(modelDescription->model_variables)) {
		if (inputVar.causality != fmi4cpp::fmi2::causality::input || !inputVar.is_integer()) {
			continue;
		}
		for (auto address : toFMUAddresses) {
			if (inputVar.name.find(address.first) == std::string::npos) {
				continue;
			}
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

void OSMP::saveToAddressMap(std::map<std::string, address> &addressMap, const std::string& name, int value) {
	//check for normal fmi variables count and valid
	if (name == "count") {
		//this->count = value;
		return;
	}
	if (name == "valid") {
		this->valid = value;
		return;
	}

	//compare return 0 -> equals
	if (0 == name.compare(name.length() - 8, 8, ".base.hi")) {
		std::string prefixWithIndex = name.substr(0, name.length() - 8);

		if (addressMap.find(prefixWithIndex) == addressMap.end()) {
			address a;
			a.addr.base.hi = value;
			a.name = prefixWithIndex;
			addressMap.insert({ prefixWithIndex, a });
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
			addressMap.insert({ prefixWithIndex, a });
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
			addressMap.insert({ prefixWithIndex, a });
		}
		else {
			addressMap.at(prefixWithIndex).size = value;
		}
	}
}

inline OSMP::OSMPFMUSlaveStateWrapper::OSMPFMUSlaveStateWrapper(std::shared_ptr<fmi4cpp::fmi2::cs_slave> slave) {
	slave->get_fmu_state(state);
	coSimSlave = slave;
}

inline OSMP::OSMPFMUSlaveStateWrapper::~OSMPFMUSlaveStateWrapper() {
	coSimSlave->free_fmu_state(state);
}

std::optional<OSMP::OSMPFMUSlaveStateWrapper> OSMP::OSMPFMUSlaveStateWrapper::tryGetStateOf(std::shared_ptr<fmi4cpp::fmi2::cs_slave> slave) {
	if (slave->get_model_description()->can_get_and_set_fmu_state) {
		return OSMP::OSMPFMUSlaveStateWrapper(slave);
	}
	return std::nullopt;
}
