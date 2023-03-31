#include "GRPCServer.h"

void GRPCServer::startServer(const bool nonBlocking)
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

void GRPCServer::stopServer()
{
	if (server)
		server->Shutdown(std::chrono::system_clock::now() + transaction_timeout);
	if (server_thread)
		server_thread->join();
	server = nullptr;
	std::cout << "Server stopped" << std::endl;
}

grpc::Status GRPCServer::SetConfig(grpc::ServerContext* context, const CoSiMa::rpc::OSMPConfig* config, CoSiMa::rpc::Status* response)
{

	OSMPSERVICEMODE mode = determineMode(config);
	std::string filename = saveFile(config, mode);

	std::string modename;
	switch (mode) {
	case FMU:
		serviceInterface = std::make_unique<OSMP>();
		modename = "OSMP";
		break;
	case PLAYBACK:
		serviceInterface = std::make_unique<Playback>();
		modename = "playback";
		break;
	case RECORD:
		serviceInterface = std::make_unique<Record>();
		modename = "record";
		break;
	}

	if (verbose) {
		std::cout << "Running in " << modename << " mode." << std::endl;
	}

	int responsevalue = serviceInterface->create(filename);
	serviceInterface->init(verbose);

	for (auto& initialparameter : config->parameter()) {
		serviceInterface->setInitialParameter(initialparameter.name(), initialparameter.value());
	}

	serviceInterface->finishInitialization();

	response->set_code(responsevalue == 0 ? CoSiMa::rpc::Ok : CoSiMa::rpc::Failed);
	return grpc::Status::OK;
}

GRPCServer::OSMPSERVICEMODE GRPCServer::determineMode(const CoSiMa::rpc::OSMPConfig* config) {
	if (config->filepath().length()) {
		if (0 == config->filepath().compare(config->filepath().length() - 3, 3, "fmu")) {
			return FMU;
		}
		return PLAYBACK;
	}
	return RECORD;
}

std::string GRPCServer::saveFile(const CoSiMa::rpc::OSMPConfig* config, GRPCServer::OSMPSERVICEMODE mode) {
	std::string filename;
	std::string file = config->binaryfile();
	if (file.length() != 0) {
		//write file
		if (mode == OSMPSERVICEMODE::FMU) { filename = FMUNAME; }
		if (mode == OSMPSERVICEMODE::PLAYBACK) { filename = CSVINPUTNAME; }
		std::ofstream binFile(filename, std::ios::binary);
		binFile.write(file.c_str(), file.size());
		binFile.close();
	}
	else {
		//read file with given path
		filename = config->filepath();
	}
	return filename;
}

grpc::Status GRPCServer::GetStringValue(grpc::ServerContext* context, const CoSiMa::rpc::String* request, CoSiMa::rpc::Bytes* response) {
	response->set_value(
		serviceInterface->readOSIMessage(request->value()));
	return grpc::Status::OK;
}

grpc::Status GRPCServer::SetStringValue(grpc::ServerContext* context, const CoSiMa::rpc::NamedBytes* request, CoSiMa::rpc::Int32* response) {
	response->set_value(
		serviceInterface->writeOSIMessage(request->name(), request->value()));
	return grpc::Status::OK;
}

grpc::Status GRPCServer::DoStep(grpc::ServerContext* context, const CoSiMa::rpc::Double* request, CoSiMa::rpc::Int32* response) {
	//divider default value: 1 
	if (divider <= doStepCounter) {
		doStepCounter = 1;
		if (verbose) {
			std::cout << "DoStep with stepsize " << request->value() << std::endl;
		}
		response->set_value(serviceInterface->doStep(request->value()));
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

grpc::Status GRPCServer::Close(grpc::ServerContext* context, const CoSiMa::rpc::Bool* request, CoSiMa::rpc::Bool* response) {
	serviceInterface->close();
	response->set_value(true);
	return grpc::Status::OK;
}
