/**
@authors German Aerospace Center: Nils Wendorff, Björn Bahn, Danny Behnecke
*/

#ifndef OSIMESSAGES_H
#define OSIMESSAGES_H

#include <variant>
#include "osi_sensorview.pb.h"
#include "osi_sensordata.pb.h"
#include "osi_sensorviewconfiguration.pb.h"
#include "osi_groundtruth.pb.h"
#include "osi_trafficcommand.pb.h"
#include "osi_trafficupdate.pb.h"

/**
* Implemented OSI Messages
*/
enum eOSIMessage {

	SensorViewMessage,
	SensorViewConfigurationMessage,
	SensorDataMessage,
	GroundTruthMessage,
	TrafficCommandMessage,
	TrafficUpdateMessage,
};

/**
* Address struct and union to convert integer in pointer and vice versa. See OSI Sensor Model Packaging Specification 
*/
struct address {
	union pointerUnion {
		struct {
			int lo;
			int hi;
		} base;
		unsigned long long address;
	} addr;
	int size;
	std::string name;
};

#endif // !OSIMESSAGES_H
