#include "Playback.h"

int Playback::create(const std::string& path) {
	filestream.open(path.c_str());
	return 0;
}

void Playback::init(bool verbose, float starttime) {
	this->verbose = verbose;
	//std::string expectedBeginning = "ts,id,h,w,l,class,vx,vy,vel,ax,ay,acc,heading,lane,reference_lane_distance,pos_in_lane,direction,leader_id,leader_speed,leader_pos,leader_gap,x_fc,y_fc,x_fl,y_fl,x_fr,y_fr,x_rc,y_rc,x_rl,y_rl,x_rr,y_rr,x,y,z";

	currentLine = parseNextLine();
	for (uint8_t index = 0; index < currentLine.size(); index++) {
		if (currentLine[index] == "ts") {
			indexTS = index;
		}
		else if (currentLine[index] == "id") {
			indexID = index;
		}
		else if (currentLine[index] == "h") {
			indexHeight = index;
		}
		else if (currentLine[index] == "w") {
			indexWidth = index;
		}
		else if (currentLine[index] == "l") {
			indexLength = index;
		}
		else if (currentLine[index] == "class") {
			indexClass = index;
		}
		else if (currentLine[index] == "vx") {
			indexVelocityX = index;
		}
		else if (currentLine[index] == "vy") {
			indexVelocityY = index;
		}
		else if (currentLine[index] == "ay") {
			indexAccelerationY = index;
		}
		else if (currentLine[index] == "ay") {
			indexAccelerationY = index;
		}
		else if (currentLine[index] == "heading") {
			indexOrientation = index;
		}
		else if (currentLine[index] == "x") {
			indexPositionX = index;
		}
		else if (currentLine[index] == "y") {
			indexPositionY = index;
		}
		else if (currentLine[index] == "z") {
			indexPositionZ = index;
		}
	}
	currentLine = parseNextLine();

	timeOffsetMicros = std::stoull(currentLine[indexTS]);
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
	while (std::stoull(currentLine[indexTS]) - timeOffsetMicros <= simulationTimeMicros) {
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
	unsigned long long id = std::stoull(values[indexID]);
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

	base->mutable_position()->set_x(std::stof(values[indexPositionX]));
	base->mutable_position()->set_y(std::stof(values[indexPositionY]));
	base->mutable_position()->set_z(std::stof(values[indexPositionZ]));
}

void Playback::close() {
	filestream.close();
	currentLine.clear();
}
