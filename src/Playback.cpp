#include "Playback.h"

int Playback::create(const std::string& path) {
	filestream.open(path.c_str());
	return 0;
}

void Playback::init(bool verbose, bool nano, float starttime) {
	this->verbose = verbose;
	this->nano = nano;

	currentLine = parseNextLine();
	for (uint8_t index = 0; index < currentLine.size(); index++) {
		if (!currentLine[index].compare("ts")) {
			indexTS = index;
		}
		else if (!currentLine[index].compare("id")) {
			indexID = index;
			idIndexSet = true;
		}
		else if (!currentLine[index].compare("h")) {
			indexHeight = index;
		}
		else if (!currentLine[index].compare("w")) {
			indexWidth = index;
		}
		else if (!currentLine[index].compare("l")) {
			indexLength = index;
		}
		else if (!currentLine[index].compare("class")) {
			indexClass = index;
		}
		else if (!currentLine[index].compare("vx")) {
			indexVelocityX = index;
		}
		else if (!currentLine[index].compare("vy")) {
			indexVelocityY = index;
		}
		else if (!currentLine[index].compare("ax")) {
			indexAccelerationX = index;
		}
		else if (!currentLine[index].compare("ay")) {
			indexAccelerationY = index;
		}
		else if (!currentLine[index].compare("heading")) {
			indexOrientation = index;
		}
		else if (!currentLine[index].compare("x")) {
			indexPositionX = index;
		}
		else if (!currentLine[index].compare("y")) {
			indexPositionY = index;
		}
		else if (!currentLine[index].compare("z")) {
			indexPositionZ = index;
		}
	}
	if (verbose) {
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
		<< "\npositionz: " << unsigned(indexPositionZ) << std::endl;
	}
	currentLine = parseNextLine();

	if (nano){
		timeOffsetMicros = std::stoull(currentLine[indexTS]) / 1000;
	} else {
		timeOffsetMicros = std::stoull(currentLine[indexTS]);
	}
}

std::vector<std::string> Playback::parseNextLine() {
	std::string line;
	std::getline(filestream, line);
	std::stringstream lineStream(line);
	std::string cell;
	std::vector<std::string> parsed;
	while (std::getline(lineStream, cell, ','))
	{
		parsed.push_back(cell);
	}
	return parsed;
}

int Playback::writeOSIMessage(const std::string& name, const std::string& value) {
	return 0;
}

int Playback::readOSIMessage(const std::string& name, std::string& message) {
	int status = 1;
	if (getMessageType(name) == eOSIMessage::TrafficUpdateMessage) {
		osi3::TrafficUpdate trafficUpdate;
		status = createTrafficUpdateMessage(trafficUpdate);
		trafficUpdate.SerializeToString(&message);
		status = 0;
	}
	//add more message types
	return status;
}

int Playback::doStep(double stepSize) {
	simulationTimeMicros += (unsigned long long)(stepSize * 1000000);
	return 0;
}

int Playback::createTrafficUpdateMessage(osi3::TrafficUpdate& trafficUpdate) {
	if (verbose) {
		std::cout << "nano:" << nano << std::endl;
		std::cout << "original:" << currentLine[indexTS] << std::endl;
		std::cout << "variante 1" << std::stoull(currentLine[indexTS]) / 1000 << std::endl;
		std::cout << "variante 2" << std::stoull(currentLine[indexTS]) << std::endl;
		std::cout << "choose" << (long)((nano ? std::stoull(currentLine[indexTS]) / 1000 : std::stoull(currentLine[indexTS]))) << std::endl;
		std::cout << "timeoffsetmicros: " << timeOffsetMicros << std::endl;
		std::cout << "difference:" << ((long)((nano ? std::stoull(currentLine[indexTS]) / 1000 : std::stoull(currentLine[indexTS]))) - timeOffsetMicros) << std::endl;
		std::cout << "simulationtimemicros" << simulationTimeMicros << std::endl;
		std::cout << "result:" << (((long)((nano ? std::stoull(currentLine[indexTS]) / 1000 : std::stoull(currentLine[indexTS]))) - timeOffsetMicros) <= simulationTimeMicros) << std::endl;
	}
	while (((long)((nano ? std::stoull(currentLine[indexTS]) / 1000 : std::stoull(currentLine[indexTS]))) - timeOffsetMicros) <= simulationTimeMicros) {
		osi3::MovingObject* movingObject = trafficUpdate.add_update();
		createMovingObject(currentLine, movingObject);
		currentLine = parseNextLine();
		if (currentLine.size() == 0) {
			if (verbose) {
				std::cout << "Reach end of file." << std::endl;
			}
			return 1;
		}
	}

	trafficUpdate.mutable_timestamp()->set_seconds((int64_t)simulationTimeMicros / 1000000);
	trafficUpdate.mutable_timestamp()->set_nanos((uint32_t)((simulationTimeMicros % 1000000) * 1000));
	if (verbose) {
		std::cout << "Traffic Update with " << trafficUpdate.update_size() << " vehicles." << std::endl;
	}
	return 0;
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

void Playback::close() {
	filestream.close();
	currentLine.clear();
}
