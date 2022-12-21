/**
@authors German Aerospace Center: Nils Wendorff, Björn Bahn, Danny Behnecke
*/

#include "catch2/catch.hpp"

#include "OSMPInterface.h"
#include "TestResourceDirectory.h"

TEST_CASE("OSMP Interface Test","[OSMPInterface]") {

	OSMPInterface source;

	CHECK(0 == source.create(testResourceDirectory + "/OSMPDummySource.fmu"));
	source.init(false);

	// OSMP-FMUs have to abide FMI, thus this variable defined as ausality="output" variability="discrete" initial="exact" has to be readable. It can return a nullpointer
	std::string serializedSensorView = source.readOSIMessage("OSMPSensorViewOut");
	CHECK(0 <= serializedSensorView.size());

	osi3::SensorView sensorView;
	sensorView.ParseFromString(serializedSensorView);

	CHECK(serializedSensorView.size() == sensorView.ByteSizeLong());

	CHECK(0 == source.doStep(0.06));

	serializedSensorView = source.readOSIMessage("OSMPSensorViewOut");
	CHECK(0 < serializedSensorView.size());

	OSMPInterface sensor;

	CHECK(0 == sensor.create(testResourceDirectory + "/OSMPDummySensor.fmu"));
	sensor.init(false);

	CHECK(0 == sensor.writeOSIMessage("OSMPSensorViewIn", serializedSensorView));

	CHECK(0 == sensor.doStep(0.06));

	std::string serializedSensorData = sensor.readOSIMessage("OSMPSensorDataOut");
	CHECK(0 < serializedSensorData.size());

	osi3::SensorData sensorData;
	sensorData.ParseFromString(serializedSensorData);

	CHECK(0 < sensorData.ByteSizeLong());
	CHECK(serializedSensorData.size() == sensorData.ByteSizeLong());
}
