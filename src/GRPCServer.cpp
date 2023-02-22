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
	bool isFMU = true;
	bool isRecorder = false;

	if (config->filepath().length() >= 5) {//a.fmu or a.csv
		if (0 == config->filepath().compare(config->filepath().length() - 3, 3, "fmu")) {
			isFMU = true;
		} else if (0 == config->filepath().compare(config->filepath().length() - 3, 3, "csv")) {
			isFMU = false;
		}
	}

	std::string filename = isFMU ? FMUNAME : CSVINPUTNAME;
	std::string file = config->binaryfile();

	if (file.length() != 0) {
		//write file
		std::ofstream binFile(filename, std::ios::binary);
		binFile.write(file.c_str(), file.size());
		binFile.close();
	}
	else {
		//no input file or model implies recording mode
		filename = LOGOUTPUTNAME;
		isRecorder = true;
		isFMU = false;
	}

	if (isFMU) {
		serviceInterface = std::make_unique<OSMP>();
		if (verbose) {
			std::cout << "Running is OSMP mode." << std::endl;
		}
	}
	else if (isRecorder) {
		serviceInterface = std::make_unique<Record>();
		if (verbose) {
			std::cout << "Running is record mode." << std::endl;
		}
	}
	else {
		serviceInterface = std::make_unique<Playback>();
		if (verbose) {
			std::cout << "Running is plaback mode." << std::endl;
		}
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
