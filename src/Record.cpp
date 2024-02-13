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

	auto file = output.find(name);
	if (file == output.end()) {
		//create ofstream, if not yet done.
		std::ofstream* logFile = new std::ofstream(name + ".osi", std::ofstream::binary);
		output.emplace(name, logFile);
		file = output.find(name);
	}
	std::ofstream* stream = file->second;

	uint32_t size = (uint32_t)value.size();
	//format: size as long, message
	stream->write((char*)&size, sizeof(size));
	*stream << value << std::flush;

	osi3::SensorView sensorView;
	if (sensorView.ParseFromString(value)) {
		if (writeThread.joinable()) {
			writeThread.join();
		}
		writeThread = std::thread(&Record::saveImage, this, sensorView, name);
	}
	return 0;
}

void Record::saveImage(const osi3::SensorView sensorView, const std::string name) {
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
		osi3::SensorViewConfiguration c;
		message = c.SerializeAsString();
	}
	return 0;
}

int Record::doStep(double stepSize) {
	return 0;
}

void Record::close() {
	if (writeThread.joinable()) {
		writeThread.join();
	}
	for (auto& file : output) {
		file.second->close();
		delete file.second;
		file.second = 0;
	}
	output.clear();
}
