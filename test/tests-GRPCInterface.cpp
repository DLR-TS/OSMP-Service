/**
@authors German Aerospace Center: Nils Wendorff, Björn Bahn, Danny Behnecke
*/

#include "catch2/catch.hpp"

#include "GRPCInterface.h"

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include "TestResourceDirectory.h"
#include "grpc_proto_files/simulation_interface/OSMPSimulationInterface.grpc.pb.h"
#include "grpc_proto_files/simulation_interface/OSMPSimulationInterface.pb.h"
#include "grpc_proto_files/simulation_interface/SimulationInterface.grpc.pb.h"
#include "grpc_proto_files/simulation_interface/SimulationInterface.pb.h"

std::unique_ptr<grpc::ClientContext> CreateDeadlinedClientContext(double transactionTimeout) {
	// context to handle a rpc call - cannot be reused
	std::unique_ptr<grpc::ClientContext> context = std::make_unique<grpc::ClientContext>();
	// double to integer conversion
	std::chrono::duration timeout = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::duration<double>(transactionTimeout));
	// set deadline to transactionTimeout seconds from now
	context->set_deadline(std::chrono::system_clock::now() + timeout);
	return context;
}


TEST_CASE("gRPC interface test","[GRPCInterface]") {
	/*
	Test public interface of GRPCInterface by starting the OSMP FMUs OSMPDummySource and OSMPDummySensor
	and feeding the output from the first to the second fmu and testing the output size of the sensor's SensorDataOut message
	*/
	double transactionTimeout = 5.0;

	std::string hostAddrSource = "localhost:51426";
	std::string hostAddrSensor = "localhost:51427";
	GRPCInterface sourceGRPCService(hostAddrSource, false);
	GRPCInterface sensorGRPCService(hostAddrSensor, false);
	sourceGRPCService.startServer(true);
	sensorGRPCService.startServer(true);
	grpc::ChannelArguments channelArgs;
	// disable client message size limits
	channelArgs.SetMaxSendMessageSize(-1);
	channelArgs.SetMaxReceiveMessageSize(-1);
	auto sourceChannel = grpc::CreateCustomChannel(hostAddrSource, grpc::InsecureChannelCredentials(), channelArgs);
	auto sensorChannel = grpc::CreateCustomChannel(hostAddrSensor, grpc::InsecureChannelCredentials(), channelArgs);
	CoSiMa::rpc::OSMPSimulationInterface::Stub sourceOSMPStub(sourceChannel);
	CoSiMa::rpc::SimulationInterface::Stub sourceSimStub(sourceChannel);
	CoSiMa::rpc::OSMPSimulationInterface::Stub sensorOSMPStub(sensorChannel);
	CoSiMa::rpc::SimulationInterface::Stub sensorSimStub(sensorChannel);
	
	CoSiMa::rpc::OSMPConfig config;
	config.set_fmupath(testResourceDirectory + "/OSMPDummySource.fmu");

	CoSiMa::rpc::Int32 response;

	auto status = sourceOSMPStub.SetConfig(CreateDeadlinedClientContext(transactionTimeout).get(), config, &response);

	CHECK(status.ok());
	CHECK(0 == response.value());

	config.set_fmupath(testResourceDirectory + "/OSMPDummySensor.fmu");
	response.Clear();

	status = sensorOSMPStub.SetConfig(CreateDeadlinedClientContext(transactionTimeout).get(), config, &response);

	CHECK(status.ok());
	CHECK(0 == response.value());

	CoSiMa::rpc::String osmpSensorViewOut;
	CoSiMa::rpc::Bytes serializedSensorView;
	osmpSensorViewOut.set_value("OSMPSensorViewOut");

	//here, returning an empty message is valid because time has not advanced, yet
	status = sourceSimStub.GetStringValue(CreateDeadlinedClientContext(transactionTimeout).get(), osmpSensorViewOut, &serializedSensorView);

	CHECK(status.ok());
	CHECK(0 <= serializedSensorView.value().size());

	CoSiMa::rpc::NamedBytes osmpSensorViewIn;
	response.Clear();
	osmpSensorViewIn.set_name("OSMPSensorViewIn");
	osmpSensorViewIn.set_value(serializedSensorView.value());

	status = sensorSimStub.SetStringValue(CreateDeadlinedClientContext(transactionTimeout).get(), osmpSensorViewIn, &response);

	CHECK(status.ok());
	CHECK(0 == response.value());

	CoSiMa::rpc::Double stepSize;
	stepSize.set_value(0.06);
	response.Clear();
	
	status = sourceSimStub.DoStep(CreateDeadlinedClientContext(transactionTimeout).get(), stepSize, &response);

	CHECK(status.ok());
	CHECK(0 == response.value());

	status = sensorSimStub.DoStep(CreateDeadlinedClientContext(transactionTimeout).get(), stepSize, &response);

	CHECK(status.ok());
	CHECK(0 == response.value());

	serializedSensorView.Clear();

	status = sourceSimStub.GetStringValue(CreateDeadlinedClientContext(transactionTimeout).get(), osmpSensorViewOut, &serializedSensorView);

	CHECK(status.ok());
	CHECK(0 < serializedSensorView.value().size());

	CoSiMa::rpc::String osmpSensorDataOut;
	CoSiMa::rpc::Bytes serializedSensorData;
	osmpSensorDataOut.set_value("OSMPSensorDataOut");

	//Input of last step was empty, thus the output is also
	status = sensorSimStub.GetStringValue(CreateDeadlinedClientContext(transactionTimeout).get(), osmpSensorDataOut, &serializedSensorData);

	CHECK(status.ok());
	CHECK(0 <= serializedSensorData.value().size());

	response.Clear();
	osmpSensorViewIn.set_value(serializedSensorView.value());

	status = sensorSimStub.SetStringValue(CreateDeadlinedClientContext(transactionTimeout).get(), osmpSensorViewIn, &response);

	CHECK(status.ok());
	CHECK(0 == response.value());

	response.clear_value();

	status = sensorSimStub.DoStep(CreateDeadlinedClientContext(transactionTimeout).get(), stepSize, &response);

	CHECK(status.ok());
	CHECK(0 == response.value());

	serializedSensorData.Clear();

	status = sensorSimStub.GetStringValue(CreateDeadlinedClientContext(transactionTimeout).get(), osmpSensorDataOut, &serializedSensorData);

	CHECK(status.ok());
	CHECK(0 < serializedSensorData.value().size());

	sourceGRPCService.stopServer();
	sensorGRPCService.stopServer();
}
