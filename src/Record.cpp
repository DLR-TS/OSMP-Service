#include "Record.h"

int Record::create(const std::string& name) {
	return 0;
}

void Record::init(bool verbose, OSMPTIMEUNIT timeunit, float starttime) {
}

int Record::writeOSIMessage(const std::string& name, const std::string& value) {
	eOSIMessage messageType = getMessageType(name);
	if (messageType == eOSIMessage::SensorViewConfigurationMessage) {
		return 0;
	}

	RecordFileName fileName;

	auto fileEntry = output.find(name);
	if (fileEntry == output.end()) {
		fileName = createCompliantNameForOSITraceFile(value, name, messageType);
		output.emplace(name, fileName);
	}
	else {
		fileName = fileEntry->second;
		RecordFileName newFileName = fileName;
		newFileName.amount++;
		std::rename(asString(fileName).c_str(), asString(newFileName).c_str());
		fileEntry->second = newFileName;
		fileName = newFileName;
	}

	std::ofstream stream = std::ofstream(asString(fileName), std::ofstream::binary | std::ofstream::app);
	uint32_t size = (uint32_t)value.size();
	stream.write((char*)&size, sizeof(size));
	stream << value << std::flush;
	stream.close();

	osi3::SensorView sensorView;
	if (messageType == eOSIMessage::SensorViewMessage) {
		sensorView.ParseFromString(value);
		if (writeThread.joinable()) {
			writeThread.join();
		}
		writeThread = std::thread(&Record::saveImage, this, name, sensorView);
	}
	return 0;
}

void Record::saveImage(const std::string& name, const osi3::SensorView& sensorView) {
	for (auto& cameraSensorView : sensorView.camera_sensor_view()) {
		if (!cameraSensorView.has_view_configuration()) {
			std::cerr << "No OSI3::CameraSensorViewConfiguration given for image!" << std::endl;
			continue;
		}
		osi3::CameraSensorViewConfiguration camConfig = cameraSensorView.view_configuration();
		std::string imageData = cameraSensorView.image_data();

		int imageWidth = camConfig.number_of_pixels_horizontal();
		int imageHeight = camConfig.number_of_pixels_vertical();

		std::vector <boost::gil::rgb8_pixel_t> pixels;
		pixels.reserve(imageWidth * imageHeight);

		for (size_t i = 0; i < imageData.size(); i += 3) {
			boost::gil::rgb8_pixel_t pixel(imageData[i], imageData[i + 1], imageData[i + 2]);
			pixels.push_back(pixel);
		}

		auto rgbView = boost::gil::interleaved_view(imageWidth, imageHeight, pixels.data(), imageWidth * sizeof(boost::gil::rgb8_pixel_t));

		std::string fileName("SensorView_" + name + "_" + formatTimeToMS(sensorView.timestamp()) + ".png");

		boost::gil::write_view(fileName, rgbView, boost::gil::png_tag());
	}
}

std::string Record::formatTimeToMS(const osi3::Timestamp& timestamp) {
	std::chrono::milliseconds timeMilliseconds(timestamp.seconds() * 1000LL + timestamp.nanos() / 1000000);
	return std::to_string(timeMilliseconds.count()) + "ms";
}

int Record::readOSIMessage(const std::string& name, std::string& message) {
	std::cout << "Nothing to read from " << name << std::endl;
	if (getMessageType(name) == eOSIMessage::SensorViewConfigurationMessage) {
		return 0;
	}
	return 1;
}

int Record::writeParameter(const std::string& name, const std::string& value) {
	return 0;
};

int Record::readParameter(const std::string& name, std::string& value) {
	return 0;
}

int Record::doStep(double stepSize) {
	return 0;
}

void Record::close() {
	if (writeThread.joinable()) {
		writeThread.join();
	}
	output.clear();
}

Record::RecordFileName Record::createCompliantNameForOSITraceFile(const std::string& message,
	const std::string& name, const eOSIMessage& messageType) {
	std::string type;
	int64_t seconds;
	osi3::InterfaceVersion version;
	switch (messageType) {
	case GroundTruthMessage: {
		type = "gt";
		osi3::GroundTruth gt;
		gt.ParseFromString(message);
		seconds = gt.timestamp().seconds();
		version.CopyFrom(gt.version());
		break;
	}
	case SensorViewMessage: {
		type = "sv";
		osi3::SensorView sv;
		sv.ParseFromString(message);
		seconds = sv.timestamp().seconds();
		version.CopyFrom(sv.version());
		break;
	}
	case SensorDataMessage: {
		type = "sd";
		osi3::SensorData sd;
		sd.ParseFromString(message);
		seconds = sd.timestamp().seconds();
		version.CopyFrom(sd.version());
		break;
	}
	case TrafficUpdateMessage: {
		type = "tu";
		osi3::TrafficUpdate tu;
		tu.ParseFromString(message);
		seconds = tu.timestamp().seconds();
		version.CopyFrom(tu.version());
		break;
	}
	case TrafficCommandMessage: {
		type = "tc";
		osi3::TrafficCommand tc;
		tc.ParseFromString(message);
		seconds = tc.timestamp().seconds();
		version.CopyFrom(tc.version());
		break;
	}
	default: {
		type = "??";
		break;
	}
	}

	Record::RecordFileName fileName;
	fileName.firstPart = getISO8601Timestamp(seconds) + "_" + type + "_" + getVersion(version) + "_" + protobufVersion + "_";
	fileName.amount = 1;
	fileName.secondPart = "_" + name + ".osi";
	return fileName;
}

std::string Record::getISO8601Timestamp(int64_t& seconds) {

	std::time_t tm = std::time(nullptr) + seconds;

	char buffer[32];
	std::strftime(buffer, sizeof(buffer), "%Y%m%dT%H%M%SZ", std::localtime(&tm));

	return std::string(buffer);
}

std::string Record::getVersion(osi3::InterfaceVersion& version) {
	std::string major = "3";
	std::string minor = "5";
	std::string patch = "0";
	if (version.has_version_major())
		major = std::to_string(version.version_major());
	if (version.has_version_minor())
		minor = std::to_string(version.version_minor());
	if (version.has_version_patch())
		patch = std::to_string(version.version_patch());

	return major + minor + patch;
}
