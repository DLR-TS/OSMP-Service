#include "catch2/catch.hpp"

#include "OSMPInterface.h"
#include "TestResourceDirectory.h"

TEST_CASE("OSMP Interface Test","[OSMPInterface]") {

	OSMPInterface source;

	CHECK(0 == source.create(testResourceDirectory + "/OSMPDummySource.fmu"));
	CHECK(0 == source.init());

	// OSMP-FMUs have to abide FMI, thus this variable defined as ausality="output" variability="discrete" initial="exact" has to be readable. It can return a nullpointer
	std::string serializedSensorView = source.read("OSMPSensorViewOut");
	CHECK(0 <= serializedSensorView.size());

	osi3::SensorView sensorView;
	sensorView.ParseFromString(serializedSensorView);

	CHECK(serializedSensorView.size() == sensorView.ByteSizeLong());

	CHECK(0 == source.doStep(0.06));

	serializedSensorView = source.read("OSMPSensorViewOut");
	CHECK(0 < serializedSensorView.size());

	OSMPInterface sensor;

	CHECK(0 == sensor.create(testResourceDirectory + "/OSMPDummySensor.fmu"));
	CHECK(0 == sensor.init());

	CHECK(0 == sensor.write("OSMPSensorViewIn", serializedSensorView));

	CHECK(0 == sensor.doStep(0.06));

	std::string serializedSensorData = sensor.read("OSMPSensorDataOut");
	CHECK(0 < serializedSensorData.size());

	osi3::SensorData sensorData;
	sensorData.ParseFromString(serializedSensorData);

	CHECK(0 < sensorData.ByteSizeLong());
	CHECK(serializedSensorData.size() == sensorData.ByteSizeLong());

}

