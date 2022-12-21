#include "GRPCInterface.h"

void GRPCInterface::startServer(const bool nonBlocking)
{
	if (server) {
		server->Shutdown(std::chrono::system_clock::now() + transaction_timeout);
	}
	grpc::ServerBuilder builder;
	grpc::reflection::InitProtoReflectionServerBuilderPlugin();
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
	
	int responsevalue = osmpInterface.create(fmu_name);
	osmpInterface.init(verbose);
	for (auto& initialparameter : config->parameter()) {
		osmpInterface.setInitialParameter(initialparameter.name(), initialparameter.value());
	}
	osmpInterface.finishInitialization();

	response->set_value(responsevalue);
	return grpc::Status::OK;
}

grpc::Status GRPCInterface::UploadFMU(grpc::ServerContext* context, const CoSiMa::rpc::FMU* request, ::CoSiMa::rpc::UploadStatus* response) {
	if (verbose) {
		std::cout << "Upload FMU: Start" << std::endl;
	}

	std::ofstream binFile(fmu_name, std::ios::binary);
	std::string fmu = request->binaryfmu();
	binFile.write(fmu.c_str(), fmu.size());
	binFile.close();

	if (verbose) {
		std::cout << "Upload FMU: Done" << std::endl;
	}

	response->set_code(CoSiMa::rpc::UploadStatusCode::Ok);
	return grpc::Status::OK;
}

grpc::Status GRPCInterface::GetStringValue(grpc::ServerContext* context, const CoSiMa::rpc::String* request, CoSiMa::rpc::Bytes* response) {
	response->set_value(
		osmpInterface.readOSIMessage(request->value()));
	return grpc::Status::OK;
}

grpc::Status GRPCInterface::SetStringValue(grpc::ServerContext* context, const CoSiMa::rpc::NamedBytes* request, CoSiMa::rpc::Int32* response) {
	response->set_value(
		osmpInterface.writeOSIMessage(request->name(), request->value()));
	return grpc::Status::OK;
}

grpc::Status GRPCInterface::DoStep(grpc::ServerContext* context, const CoSiMa::rpc::Double* request, CoSiMa::rpc::Int32* response) {
	//divider default value: 1 
	if (divider <= doStepCounter) {
		doStepCounter = 1;
		response->set_value(osmpInterface.doStep(request->value()));
		return grpc::Status::OK;
	}
	else {
		doStepCounter++;
		if (verbose) {
			std::cout << "Skipped DoStep. Counter: " << doStepCounter << " Divider: " << divider << std::endl;
		}
		return grpc::Status::OK;
	}
}
