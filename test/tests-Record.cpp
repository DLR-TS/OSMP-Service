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

std::string findFileWithExtension(const std::string& extension) {
	for (const auto& entry : std::filesystem::directory_iterator(".")) {
		if (entry.path().extension() == extension) {
			return entry.path().filename().string();
		}
	}
	return "";
}

bool endsWith(const std::string& fullString, const std::string& ending) {
	if (fullString.length() >= ending.length()) {
		return (fullString.compare(fullString.length() - ending.length(), ending.length(), ending) == 0);
	}
	else {
		return false;
	}
}

TEST_CASE("Record Test write SensorData Message") {
	//setup with no osi files in directory
	std::string fileNameEnd1 = "Z_sd_350_3021009_1_SensorData.osi";
	std::string fileNameEnd2 = "Z_sd_350_3021009_2_SensorData.osi";
	do {
		std::remove(findFileWithExtension(".osi").c_str());
	} while (findFileWithExtension(".osi") != "");

	Record r;
	osi3::SensorData sd;

	int success = r.writeOSIMessage("SensorData", sd.SerializeAsString());
	CHECK(success == 0);

	std::string fileName_1 = findFileWithExtension(".osi");
	CHECK(endsWith(fileName_1, fileNameEnd1));
	std::ifstream file(fileName_1.c_str(), std::ifstream::binary);
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

	std::string fileName_2 = findFileWithExtension(".osi");
	CHECK(endsWith(fileName_2, fileNameEnd2));

	std::ifstream file2(fileName_2.c_str(), std::ifstream::binary);

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
	//clean up
	std::remove(findFileWithExtension(".osi").c_str());
}

TEST_CASE("Record Test write png") {
	//setup with no png files in directory
	do {
		std::remove(findFileWithExtension(".png").c_str());
	} while (findFileWithExtension(".png") != "");

	Record r;
	osi3::SensorView sv;
	auto camsv = sv.add_camera_sensor_view();
	camsv->mutable_view_configuration()->set_number_of_pixels_horizontal(1);
	camsv->mutable_view_configuration()->set_number_of_pixels_vertical(1);
	camsv->mutable_image_data()->append("255");
	camsv->mutable_image_data()->append("0");
	camsv->mutable_image_data()->append("100");
	r.saveImage("SensorView", sv);

	std::string fileName = findFileWithExtension(".png");
	CHECK(fileName != "");

	std::ifstream file(fileName.c_str(), std::ifstream::binary);
	if (file) {
		file.seekg(0, std::ios::end);
		std::streampos fileSize = file.tellg();
		file.seekg(0, std::ios::beg);
		CHECK(fileSize == 69);
		file.close();
	}
	//clean up
	std::remove(findFileWithExtension(".png").c_str());
}
