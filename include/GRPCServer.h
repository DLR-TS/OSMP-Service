/**
@authors German Aerospace Center: Nils Wendorff, Bj�rn Bahn, Danny Behnecke
*/

#ifndef GRPCSERVER_H
#define GRPCSERVER_H

#include <string>
#include <filesystem>
#include <fstream>

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

#include "OSMP.h"
#include "Record.h"
#include "Playback.h"

#include "grpc_proto_files/simulation_interface/SimulationInterface.pb.h"
#include "grpc_proto_files/simulation_interface/SimulationInterface.grpc.pb.h"
#include "grpc_proto_files/simulation_interface/OSMPSimulationInterface.pb.h"
#include "grpc_proto_files/simulation_interface/OSMPSimulationInterface.grpc.pb.h"

class GRPCServer: public CoSiMa::rpc::SimulationInterface::Service, public CoSiMa::rpc::OSMPSimulationInterface::Service
{

private:
	const std::string server_address;
	const std::chrono::milliseconds transaction_timeout;
	const bool verbose;
	const int divider;
	int doStepCounter = 1;

	const std::string FMUNAME = "OSMP-FMU.fmu";
	const std::string CSVINPUTNAME = "input.csv";
	const std::string LOGOUTPUTNAME = "output.log";

	std::shared_ptr<grpc::Server> server;
	std::unique_ptr<std::thread> server_thread;

	std::unique_ptr<ServiceInterface> serviceInterface;

public:
	GRPCServer(std::string server_address, bool verbose, int divider = 1) : server_address(server_address), verbose(verbose), divider(divider), transaction_timeout(std::chrono::milliseconds(5000)) {};
	void startServer(const bool nonBlocking = false);
	void stopServer();

	virtual grpc::Status SetConfig(grpc::ServerContext* context, const CoSiMa::rpc::OSMPConfig* config, CoSiMa::rpc::Status* response) override;
	virtual grpc::Status GetStringValue(grpc::ServerContext* context, const CoSiMa::rpc::String* request, CoSiMa::rpc::Bytes* response) override;
	virtual grpc::Status SetStringValue(grpc::ServerContext* context, const CoSiMa::rpc::NamedBytes* request, CoSiMa::rpc::Int32* response) override;
	virtual grpc::Status DoStep(grpc::ServerContext* context, const CoSiMa::rpc::Double* request, CoSiMa::rpc::Int32* response) override;

};
#endif //!GRPCSERVER_H
