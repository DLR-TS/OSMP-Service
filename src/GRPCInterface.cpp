#include "GRPCInterface.h"

void GRPCInterface::startServer(const bool nonBlocking)
{
	if (server) {
		server->Shutdown(std::chrono::system_clock::now() + transaction_timeout);
	}
	grpc::ServerBuilder builder;
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(static_cast<CoSiMa::rpc::SimulationInterface::Service*>(this));
	builder.RegisterService(static_cast<CoSiMa::rpc::OSMPSimulationInterface::Service*>(this));
	builder.SetMaxReceiveMessageSize(-1);
	builder.SetMaxSendMessageSize(-1);
	server = builder.BuildAndStart();
	if (!nonBlocking) {
		server->Wait();
	}
	else {
		server_thread = std::make_unique<std::thread>(&grpc::Server::Wait, server);
	}
}

void GRPCInterface::stopServer()
{
	if (server)
		server->Shutdown(std::chrono::system_clock::now() + transaction_timeout);
	if (server_thread)
		server_thread->join();
	server = nullptr;
	std::cout << "Server stopped" << std::endl;
}

grpc::Status GRPCInterface::SetConfig(grpc::ServerContext* context, const CoSiMa::rpc::OSMPConfig* config, CoSiMa::rpc::Int32* response)
{
	if (config->fmupath().size() >= 5) {//a.fmu
		//try to find local fmu
		fmu_name = config->fmupath();
		if (verbose) {
			std::cout << "Path to FMU received instead of FMU itself: " << fmu_name << std::endl;
		}
	}
	int i_response = osmpInterface.create(fmu_name);
	i_response += osmpInterface.init(verbose);
	response->set_value(i_response);
	//set parameters
	std::vector<std::pair<std::string, std::string>> parameters{};
	for (int i = 0; i < config->parameter_size(); i++) {
		const auto& parameter = config->parameter(i);
		if (verbose) {
			std::cout << parameter.name() << parameter.value() << std::endl;
		}
		parameters.push_back(std::make_pair(parameter.name(), parameter.value()));
	}
	osmpInterface.setParameter(parameters);
	std::cout << i_response << std::endl;
	return grpc::Status::OK;
}

grpc::Status GRPCInterface::UploadFMU(grpc::ServerContext* context, const CoSiMa::rpc::FMU* request, ::CoSiMa::rpc::UploadStatus* response) {
	if (verbose) {
		std::cout << "Write FMU." << std::endl;
	}

	std::ofstream binFile(fmu_name, std::ofstream::out | std::ios::binary);
	int chunksize = request->chunk_size();
	for (int i = 0; i < chunksize; i++) {
		const char* c = request->chunk(i).content().c_str();
		binFile.put(*c);
	}
	binFile.close();

	return grpc::Status::OK;
}

grpc::Status GRPCInterface::GetStringValue(grpc::ServerContext* context, const CoSiMa::rpc::String* request, CoSiMa::rpc::Bytes* response) {
	response->set_value(
		osmpInterface.read(request->value()));
	return grpc::Status::OK;
};

grpc::Status GRPCInterface::SetStringValue(grpc::ServerContext* context, const CoSiMa::rpc::NamedBytes* request, CoSiMa::rpc::Int32* response) {
	response->set_value(
		osmpInterface.write(request->name(), request->value()));
	return grpc::Status::OK;
};

grpc::Status GRPCInterface::DoStep(grpc::ServerContext* context, const CoSiMa::rpc::Double* request, CoSiMa::rpc::Int32* response) {
std::cout << "dostep" << std::endl;
	response->set_value(
		osmpInterface.doStep(request->value()));
	return grpc::Status::OK;
};
