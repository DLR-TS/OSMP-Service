#include "Playback.h"

int Playback::create(const std::string& path) {
	filestream.open(path.c_str());
	return 0;
}

void Playback::init(bool verbose, OSMPTIMEUNIT timeunit, float starttime) {
	this->verbose = verbose;
	bool trafficCommandProvider = false;

	currentLine = parseNextLine();
	for (uint8_t index = 0; index < currentLine.size(); index++) {

		//fix for csv generated on windows with \r\n at the end of line
		//std::string::rfind(name, 0) == 0
		//instead of
		//!std::string::compare(name)

		//TrafficUpdate
		if (currentLine[index].rfind("ts", 0) == 0) {
			indexTS = index;
		}
		else if (currentLine[index].rfind("id", 0) == 0) {
			indexID = index;
			idIndexSet = true;
		}
		else if (currentLine[index].compare("h") == 0) {//explicit compare because of "heading"
			indexHeight = index;
		}
		else if (currentLine[index].rfind("w", 0) == 0) {
			indexWidth = index;
		}
		else if (currentLine[index].rfind("l", 0) == 0) {
			indexLength = index;
		}
		else if (currentLine[index].rfind("class", 0) == 0) {
			indexClass = index;
		}
		else if (currentLine[index].rfind("vx", 0) == 0) {
			indexVelocityX = index;
		}
		else if (currentLine[index].rfind("vy", 0) == 0) {
			indexVelocityY = index;
		}
		else if (currentLine[index].rfind("ax", 0) == 0) {
			indexAccelerationX = index;
		}
		else if (currentLine[index].rfind("ay", 0) == 0) {
			indexAccelerationY = index;
		}
		else if (currentLine[index].rfind("heading", 0) == 0) {
			indexOrientation = index;
		}
		else if (currentLine[index].rfind("x", 0) == 0) {
			indexPositionX = index;
		}
		else if (currentLine[index].rfind("y", 0) == 0) {
			indexPositionY = index;
		}
		else if (currentLine[index].rfind("z", 0) == 0) {
			indexPositionZ = index;
		}
		else if (currentLine[index].rfind("model_reference", 0) == 0) {
			indexModelReference = index;
		}
		//TrafficCommand
		else if (currentLine[index].rfind("LongitudinalDistanceAction_Distance", 0) == 0) {
			indexLongitudinalDistanceActionDistance = index;
			trafficCommandProvider = true;
		}
		else if (currentLine[index].rfind("SpeedAction_AbsoluteTargetSpeed", 0) == 0) {
			indexSpeedActionAbsoluteTargetSpeed = index;
			trafficCommandProvider = true;
		}
	}
	if (verbose && !trafficCommandProvider) {
		std::cout << "Configuration of indexes: "
			<< "\ntimestamp: " << unsigned(indexTS)
			<< "\nindex: " << unsigned(indexID)
			<< "\nheight: " << unsigned(indexHeight)
			<< "\nwidht: " << unsigned(indexWidth)
			<< "\nlength: " << unsigned(indexLength)
			<< "\nclass: " << unsigned(indexClass)
			<< "\nvx: " << unsigned(indexVelocityX)
			<< "\nvy: " << unsigned(indexVelocityY)
			<< "\nax: " << unsigned(indexAccelerationX)
			<< "\nay: " << unsigned(indexAccelerationY)
			<< "\nheading: " << unsigned(indexOrientation)
			<< "\npositionx: " << unsigned(indexPositionX)
			<< "\npositiony: " << unsigned(indexPositionY)
			<< "\npositiony: " << unsigned(indexPositionZ)
			<< "\nmodel_reference: " << unsigned(indexModelReference) << std::endl;
	}
	else if (verbose && trafficCommandProvider) {
		std::cout << "Configuration of indexes: "
			<< "\nindex: " << unsigned(indexID)
			<< "\nLongitudinalDistanceActionDistance: " << unsigned(indexLongitudinalDistanceActionDistance)
			<< "\nSpeedAction_AbsoluteTargetSpeed: " << unsigned(indexSpeedActionAbsoluteTargetSpeed) << std::endl;
	}
	currentLine = parseNextLine();

	if (!trafficCommandProvider) {
		timeOffsetMicros = determineTimeOffset(timeunit, currentLine[indexTS]);
	}
}

std::vector<std::string> Playback::parseNextLine() {
	std::string line;
	std::vector<std::string> parsed;

	if (filestream.eof()) {
		return parsed;
	}
	std::getline(filestream, line);
	    if (filestream.fail()) {
		return parsed;
	}
	std::stringstream lineStream(line);
	std::string cell;

	while (std::getline(lineStream, cell, ','))
	{
		parsed.push_back(cell);
	}
	return parsed;
}

unsigned long long Playback::determineTimeOffset(OSMPTIMEUNIT& timeunit, std::string& timestamp) {
	unsigned long long time = std::stoull(currentLine[indexTS]);
	unsigned long long timeOffset; //in microseconds

	if (timeunit == OSMPTIMEUNIT::UNSPECIFIED) {
		std::cout << "Attention! The timestamp unit will be definied automatically!" << std::endl;
		if (time >= 1600000000000000000u) { //September 2020 in nanoseconds
			timeunit = OSMPTIMEUNIT::NANO;
			std::cout << "Using nanoseconds as interpretation of timestamp.";
		} else if (time >= 1600000000000000u){ //September 2020 in microseconds
			timeunit = OSMPTIMEUNIT::MICRO;
			std::cout << "Using microseconds as interpretation of timestamp.";
		} else {
			timeunit = OSMPTIMEUNIT::MILLI;
			std::cout << "Using milliseconds as interpretation of timestamp.";
		}
		std::cout << " You can provide a timestamp interpretation by runtimeparameter." << std::endl;
	}

	switch(timeunit) {
	case OSMPTIMEUNIT::NANO:
		timeOffset = time / 1000;
		break;
	case  OSMPTIMEUNIT::MICRO:
		timeOffset = time;
		break;
	case OSMPTIMEUNIT::MILLI:
		timeOffset = time * 1000;
		break;
	}
	return timeOffset;
}

int Playback::writeOSIMessage(const std::string& name, const std::string& value) {
	return 0;
}

int Playback::readOSIMessage(const std::string& name, std::string& message) {
	int status = 1;
	auto messageType = getMessageType(name);
	switch (messageType) {
		case TrafficUpdateMessage:
		{
			osi3::TrafficUpdate trafficUpdate;
			status = createTrafficUpdateMessage(trafficUpdate);
			trafficUpdate.SerializeToString(&message);
			break;
		}
		case SensorViewConfigurationMessage:
		{
			status = 0;
			break;
		}
		case TrafficCommandMessage:
			//support for only one traffic command message
			if (trafficCommandComputed) {
				message = trafficCommandString;
				break;
			}
			osi3::TrafficCommand trafficCommand;
			status = createTrafficCommandMessage(trafficCommand);
			trafficCommand.SerializeToString(&trafficCommandString);
			message = trafficCommandString;
			break;
	}
	//add more message types
	return status;
}

int Playback::writeParameter(const std::string& name, const std::string& value) {
	return 0;
};

int Playback::readParameter(const std::string& name, std::string& value) {
	return 0;
}

int Playback::doStep(double stepSize) {
	if (firstDoStep) {
		firstDoStep = false;
		return 0;
	}
	simulationTimeMicros += (unsigned long long)(stepSize * 1000000);
	return 0;
}

int Playback::createTrafficUpdateMessage(osi3::TrafficUpdate& trafficUpdate) {
	if (currentLine.size() == 0) {
		if (verbose) {
			std::cout << "Reach end of file." << std::endl;
		}
		return 1;
	}

	if (verbose) {
		std::cout << "timeunit:" << timeunit << std::endl;
		std::cout << "original:" << currentLine[indexTS] << std::endl;
		std::cout << "variante 1" << std::stoull(currentLine[indexTS]) / 1000 << std::endl;
		std::cout << "variante 2" << std::stoull(currentLine[indexTS]) << std::endl;
		std::cout << "choose" << (long)((timeunit == OSMPTIMEUNIT::NANO ? std::stoull(currentLine[indexTS]) / 1000 : std::stoull(currentLine[indexTS]))) << std::endl;
		std::cout << "timeoffsetmicros: " << timeOffsetMicros << std::endl;
		std::cout << "difference:" << ((long)((timeunit == OSMPTIMEUNIT::NANO ? std::stoull(currentLine[indexTS]) / 1000 : std::stoull(currentLine[indexTS]))) - timeOffsetMicros) << std::endl;
		std::cout << "simulationtimemicros: " << simulationTimeMicros << std::endl;
		std::cout << "result:" << (((long)((timeunit == OSMPTIMEUNIT::NANO ? std::stoull(currentLine[indexTS]) / 1000 : std::stoull(currentLine[indexTS]))) - timeOffsetMicros) <= simulationTimeMicros) << std::endl;
	}
	while (lineInTimestep()) {
		osi3::MovingObject* movingObject = trafficUpdate.add_update();
		createMovingObject(currentLine, movingObject);
		currentLine = parseNextLine();
	}

	trafficUpdate.mutable_timestamp()->set_seconds((int64_t)simulationTimeMicros / 1000000);
	trafficUpdate.mutable_timestamp()->set_nanos((uint32_t)((simulationTimeMicros % 1000000) * 1000));
	if (verbose) {
		std::cout << "Traffic Update with " << trafficUpdate.update_size() << " vehicles." << std::endl;
	}
	return 0;
}

bool Playback::lineInTimestep() {
	if (currentLine.size() == 0) {
		return false;
	}
	unsigned long long ts;
	switch (timeunit) {
		case OSMPTIMEUNIT::NANO:
			ts = std::stoull(currentLine[indexTS]) / 1000l;
		break;
		case OSMPTIMEUNIT::MICRO:
			ts = std::stoull(currentLine[indexTS]);
		break;
		case OSMPTIMEUNIT::MILLI:
			ts = std::stoull(currentLine[indexTS]) * 1000l;
		break;
	}
	unsigned long long simulationTs = ts - timeOffsetMicros;
	if (simulationTs <= simulationTimeMicros) {
		return true;
	} else {
		return false;
	}
}

void Playback::createMovingObject(const std::vector<std::string>& values, osi3::MovingObject* movingObject) {
	//ts,id,h,w,l,class,vx,vy,vel,ax,ay,acc,heading,  (index 0-12)
	//lane,reference_lane_distance,pos_in_lane,direction,leader_id,leader_speed,leader_pos,leader_gap, (index 13-20)
	//x_fc,y_fc,x_fl,y_fl,x_fr,y_fr,x_rc,y_rc,x_rl,y_rl,x_rr,y_rr,x,y,z (index 21-35)
	unsigned long long id = 0;
	if (idIndexSet) {
		id = std::stoull(values[indexID]);
	} else {
		id = 1337;
	}

	movingObject->mutable_id()->set_value((uint64_t)id);
	std::string model_ref = values[indexModelReference];
	if (!model_ref.empty() && model_ref[model_ref.size() - 1] == '\r') {
		model_ref.erase(model_ref.size() - 1);
	}
	movingObject->set_model_reference(model_ref);
	osi3::BaseMoving* base = movingObject->mutable_base();

	base->mutable_dimension()->set_height(std::stod(values[indexHeight]));
	base->mutable_dimension()->set_width(std::stod(values[indexWidth]));
	base->mutable_dimension()->set_length(std::stod(values[indexLength]));

	if (values[indexClass] == "passenger_car") {
		movingObject->mutable_vehicle_classification()->set_type(osi3::MovingObject::VehicleClassification::TYPE_MEDIUM_CAR);
	}
	else if (values[indexClass] == "truck") {
		movingObject->mutable_vehicle_classification()->set_type(osi3::MovingObject::VehicleClassification::TYPE_HEAVY_TRUCK);
	}

	base->mutable_velocity()->set_x(std::stod(values[indexVelocityX]));
	base->mutable_velocity()->set_y(std::stod(values[indexVelocityY]));
	base->mutable_acceleration()->set_x(std::stod(values[indexAccelerationX]));
	base->mutable_acceleration()->set_y(std::stod(values[indexAccelerationY]));
	base->mutable_orientation()->set_yaw(std::stof(values[indexOrientation]) * (M_PI / 180));

	base->mutable_position()->set_x(std::stod(values[indexPositionX]));
	base->mutable_position()->set_y(std::stod(values[indexPositionY]));
	base->mutable_position()->set_z(std::stod(values[indexPositionZ]));
}

int Playback::createTrafficCommandMessage(osi3::TrafficCommand &trafficCommand) {
	trafficCommand.mutable_traffic_participant_id()->set_value(std::stoull(currentLine[indexID]));
	osi3::TrafficAction* action = trafficCommand.add_action();
	action->mutable_longitudinal_distance_action()->set_distance(std::stod(currentLine[indexLongitudinalDistanceActionDistance]));
	action = trafficCommand.add_action();
	action->mutable_speed_action()->set_absolute_target_speed(std::stod(currentLine[indexSpeedActionAbsoluteTargetSpeed]));
}


void Playback::close() {
	filestream.close();
	currentLine.clear();
}
