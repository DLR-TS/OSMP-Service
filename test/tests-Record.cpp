/**
@authors German Aerospace Center: Björn Bahn
*/

#include "catch2/catch.hpp"

#include "Record.h"
#include "TestResourceDirectory.h"

TEST_CASE("Record Test dostep") {
	Record r;
	int success = r.doStep(1);
	CHECK(0 == success);
}

TEST_CASE("Record Test close without writing before") {
	Record r;
	r.close();
	CHECK(0 == 0);
}

TEST_CASE("Record Test create") {
	Record r;
	int success = r.create("test");
	CHECK(0 == success);
}

TEST_CASE("Record Test init shall do nothing") {
	Record r;
	r.init(true, OSMPTIMEUNIT::UNSPECIFIED);
	CHECK(0 == 0);
}

TEST_CASE("Record Test set inital parameter shall do nothing") {
	Record r;
	r.setInitialParameter("a","b");
	CHECK(0 == 0);
}

TEST_CASE("Record Test finish Initialization shall do nothing") {
	Record r;
	r.finishInitialization();
	CHECK(0 == 0);
}

TEST_CASE("Record Test read OSI message other than SensorViewConfiguration shall return 1") {
	Record r;
	std::string message;
	int success = r.readOSIMessage("SensorView", message);
	CHECK(success == 1);
}

TEST_CASE("Record Test read OSI message SensorViewConfiguration shall return 1") {
	Record r;
	std::string message;
	int success = r.readOSIMessage("SensorViewConfig", message);
	CHECK(success == 0);
}

TEST_CASE("Record Test write OSI Message shall not write SensorViewConfig since it is not a sensor") {
	Record r;
	int success = r.writeOSIMessage("SensorViewConfig", "");
	CHECK(success == 0);
}

//TODO: Add more complex tests for writeOSIMessage()
