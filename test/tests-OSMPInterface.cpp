/**
@authors German Aerospace Center: Nils Wendorff, Björn Bahn, Danny Behnecke
*/

#include "catch2/catch.hpp"

#include "OSMP.h"
#include "TestResourceDirectory.h"

TEST_CASE("OSMP Interface Test", "[OSMPInterface]") {

	OSMP source;

	CHECK(0 == source.create(testResourceDirectory + "/OSMPDummySource.fmu"));
	source.init(false, OSMPTIMEUNIT::UNSPECIFIED);

	// OSMP-FMUs have to abide FMI, thus this variable defined as ausality="output" variability="discrete" initial="exact" has to be readable. It can return a nullpointer
	std::string serializedSensorView;
	int success = source.readOSIMessage("OSMPSensorViewOut", serializedSensorView);
	CHECK(0 == success);
	CHECK(0 <= serializedSensorView.size());

	osi3::SensorView sensorView;
	sensorView.ParseFromString(serializedSensorView);

	CHECK(serializedSensorView.size() == sensorView.ByteSizeLong());

	CHECK(0 == source.doStep(0.06));

	success = source.readOSIMessage("OSMPSensorViewOut", serializedSensorView);
	CHECK(0 == success);
	CHECK(0 < serializedSensorView.size());

	OSMP sensor;

	CHECK(0 == sensor.create(testResourceDirectory + "/OSMPDummySensor.fmu"));
	sensor.init(false, OSMPTIMEUNIT::UNSPECIFIED);

	CHECK(0 == sensor.writeOSIMessage("OSMPSensorViewIn", serializedSensorView));

	CHECK(0 == sensor.doStep(0.06));

	std::string serializedSensorData;
	success = sensor.readOSIMessage("OSMPSensorDataOut", serializedSensorData);
	CHECK(0 == success);
	CHECK(0 < serializedSensorData.size());

	osi3::SensorData sensorData;
	sensorData.ParseFromString(serializedSensorData);

	CHECK(0 < sensorData.ByteSizeLong());
	CHECK(serializedSensorData.size() == sensorData.ByteSizeLong());
}
