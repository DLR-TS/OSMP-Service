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

TEST_CASE("Record Test write OSI Message shall not write2 SensorViewConfig since it is not a sensor") {
	std::string filename1 = "_sd_350_3021009_1_SensorData.osi";
	std::string filename2 = "_sd_350_3021009_2_SensorData.osi";
	std::remove(filename1.c_str());
	std::remove(filename2.c_str());

	Record r;
	osi3::SensorData sd;

	int success = r.writeOSIMessage("SensorData", sd.SerializeAsString());
	CHECK(success == 0);

	std::ifstream file(filename1.c_str(), std::ifstream::binary);
	if (file) {
		file.seekg(0, std::ios::end);
		std::streampos fileSize = file.tellg();
		file.seekg(0, std::ios::beg);
		CHECK(fileSize == 4);
		file.close();
	}
	else {
		std::cout << "Could not open file initially" << std::endl;
		CHECK(false);
	}

	success = r.writeOSIMessage("SensorData", sd.SerializeAsString());
	CHECK(success == 0);
	std::ifstream file2(filename2.c_str(), std::ifstream::binary);

	if (file2) {
		file2.seekg(0, std::ios::end);
		std::streampos fileSize = file2.tellg();
		file2.seekg(0, std::ios::beg);
		CHECK(fileSize == 8);
		file2.close();
	}
	else {
		std::cout << "Could not open file after appending" << std::endl;
		CHECK(false);
	}
}

//TODO: Add more complex tests for writeOSIMessage()
