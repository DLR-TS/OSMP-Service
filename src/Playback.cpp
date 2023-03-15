#include "Playback.h"

int Playback::create(const std::string& path) {
	std::ifstream data(path.c_str());
	std::string line;
	std::getline(data, line);
	std::string expectedBeginning = "ts,id,h,w,l,class,vx,vy,vel,ax,ay,acc,heading,lane,reference_lane_distance,pos_in_lane,direction,leader_id,leader_speed,leader_pos,leader_gap,x_fc,y_fc,x_fl,y_fl,x_fr,y_fr,x_rc,y_rc,x_rl,y_rl,x_rr,y_rr,x,y,z";
	if (line.rfind(expectedBeginning, 0)) {
		std::cout << "Expected different csv header.\n"
			<< "Is:    " << line << line.size() << "\nShall: " << expectedBeginning << expectedBeginning.size() << std::endl;
		std::exit(0);
	}
	while (std::getline(data, line))
	{
		std::stringstream lineStream(line);
		std::string cell;
		std::vector<std::string> parsedRow;
		while (std::getline(lineStream, cell, ','))
		{
			parsedRow.push_back(cell);
		}

		parsedCsv.push(parsedRow);
	}

	std::cout << "Parsed " << parsedCsv.size() << " lines of csv." << std::endl;

	return 0;
}

void Playback::init(bool verbose, float starttime) {
	this->verbose = verbose;

	timeOffsetMicros = std::stoull(parsedCsv.front()[0]);
}

int Playback::writeOSIMessage(const std::string& name, const std::string& value) {
	return 0;
}

std::string Playback::readOSIMessage(const std::string& name) {
	std::string message;
	if (parsedCsv.size() == 0) {
		std::cout << "End of file. Stop OSMP Service." << std::endl;
		std::exit(0);
		//osi3::TrafficUpdate trafficUpdate;
		//trafficUpdate.SerializeToString(&message);
		//return message;
	}
	if (verbose) {
		std::cout << "Remaining entries " << parsedCsv.size() << std::endl;
	}
	if (getMessageType(name) == eOSIMessage::TrafficUpdateMessage) {
		createTrafficUpdateMessage().SerializeToString(&message);
	}
	//add more message types
	return message;
}

int Playback::doStep(double stepSize) {
	simulationTimeMicros += (unsigned long long)(stepSize * 1000000);
	return 0;
}

osi3::TrafficUpdate Playback::createTrafficUpdateMessage() {
	osi3::TrafficUpdate trafficUpdate;
	while (!parsedCsv.empty() && std::stoull(parsedCsv.front()[0]) - timeOffsetMicros <= simulationTimeMicros) {
		osi3::MovingObject* movingObject = trafficUpdate.add_update();
		createMovingObject(parsedCsv.front(), movingObject);
		parsedCsv.pop();
	}

	trafficUpdate.mutable_timestamp()->set_seconds((int64_t)simulationTimeMicros / 1000000);
	trafficUpdate.mutable_timestamp()->set_nanos((uint32_t)((simulationTimeMicros % 1000000) * 1000));
	if (verbose) {
		std::cout << "Send Traffic Update with " << trafficUpdate.update_size() << " updates." << std::endl;
	}
	return trafficUpdate;
}

void Playback::createMovingObject(const std::vector<std::string>& values, osi3::MovingObject* movingObject) {
	//ts,id,h,w,l,class,vx,vy,vel,ax,ay,acc,heading,  (index 0-12)
	//lane,reference_lane_distance,pos_in_lane,direction,leader_id,leader_speed,leader_pos,leader_gap, (index 13-20)
	//x_fc,y_fc,x_fl,y_fl,x_fr,y_fr,x_rc,y_rc,x_rl,y_rl,x_rr,y_rr,x,y,z (index 21-35)
	unsigned long long id = std::stoull(values[1]);
	movingObject->mutable_id()->set_value((uint64_t)id);
	osi3::BaseMoving* base = movingObject->mutable_base();

	base->mutable_dimension()->set_height(std::stod(values[2]));
	base->mutable_dimension()->set_width(std::stod(values[3]));
	base->mutable_dimension()->set_length(std::stod(values[4]));

	if (values[5] == "passenger_car") {
		movingObject->mutable_vehicle_classification()->set_type(osi3::MovingObject::VehicleClassification::TYPE_MEDIUM_CAR);
	}
	else if (values[5] == "truck") {
		movingObject->mutable_vehicle_classification()->set_type(osi3::MovingObject::VehicleClassification::TYPE_HEAVY_TRUCK);
	}

	base->mutable_velocity()->set_x(std::stod(values[6]));
	base->mutable_velocity()->set_y(std::stod(values[7]));
	base->mutable_acceleration()->set_x(std::stod(values[9]));
	base->mutable_acceleration()->set_y(std::stod(values[10]));
	base->mutable_orientation()->set_yaw(std::stof(values[12]) * (M_PI / 180));

	base->mutable_position()->set_x(std::stof(values[33]));
	base->mutable_position()->set_y(std::stof(values[34]));
	base->mutable_position()->set_z(std::stof(values[35]));
}
