/**
@authors German Aerospace Center: Nils Wendorff, Björn Bahn, Danny Behnecke
*/

#ifndef GRPCINTERFACE_H
#define GRPCINTERFACE_H

#include <string>
#include <filesystem>
#include <fstream>

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>

#include "OSMPInterface.h"

#include "grpc_proto_files/simulation_interface/SimulationInterface.pb.h"
#include "grpc_proto_files/simulation_interface/SimulationInterface.grpc.pb.h"
#include "grpc_proto_files/simulation_interface/OSMPSimulationInterface.pb.h"
#include "grpc_proto_files/simulation_interface/OSMPSimulationInterface.grpc.pb.h"

class GRPCInterface: public CoSiMa::rpc::SimulationInterface::Service, public CoSiMa::rpc::OSMPSimulationInterface::Service
{

private:
	const std::string server_address;
	const std::chrono::milliseconds transaction_timeout;
	const bool verbose;

	std::string fmu_name = "OSMP-FMU.fmu";

	std::shared_ptr<grpc::Server> server;
	std::unique_ptr<std::thread> server_thread;

	OSMPInterface osmpInterface;

public:
	GRPCInterface(std::string server_address, bool verbose) : server_address(server_address), verbose(verbose), transaction_timeout(std::chrono::milliseconds(5000)) {};
	void startServer(const bool nonBlocking = false);
	void stopServer();

	virtual grpc::Status SetConfig(grpc::ServerContext* context, const CoSiMa::rpc::OSMPConfig* config, CoSiMa::rpc::Int32* response) override;
	virtual grpc::Status UploadFMU(grpc::ServerContext* context, const CoSiMa::rpc::FMU* request, ::CoSiMa::rpc::UploadStatus* response) override;
	virtual grpc::Status GetStringValue(grpc::ServerContext* context, const CoSiMa::rpc::String* request, CoSiMa::rpc::Bytes* response) override;
	virtual grpc::Status SetStringValue(grpc::ServerContext* context, const CoSiMa::rpc::NamedBytes* request, CoSiMa::rpc::Int32* response) override;
	virtual grpc::Status DoStep(grpc::ServerContext* context, const CoSiMa::rpc::Double* request, CoSiMa::rpc::Int32* response) override;

};
#endif //!GRPCINTERFACE_H
