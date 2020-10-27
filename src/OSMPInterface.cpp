#include "OSMPInterface.h"

int OSMPInterface::create(std::string path) {
	std::unique_ptr<fmi4cpp::fmi2::fmu> fmu = std::make_unique<fmi4cpp::fmi2::fmu>(path);
	if (!fmu->supports_cs()) {
		// FMU contains no cs model
		return 216373;
	}

	// load co-simulation description from FMU
	coSimFMU = fmu->as_cs_fmu();
	auto modelDescription = coSimFMU->get_model_description();
	return 0;
}

int OSMPInterface::init(float starttime) {
	//Instance name cannot be set with FMU4cpp. The model identifier is used automatically instead
	coSimSlave = coSimFMU->new_instance();

	coSimSlave->setup_experiment((fmi2Real)starttime);
	coSimSlave->enter_initialization_mode();
	coSimSlave->exit_initialization_mode();
	return 0;
}

int OSMPInterface::read() {
	return 0;
}
int OSMPInterface::write() {
	return 0;
}
int OSMPInterface::close() {
	return 0;
}