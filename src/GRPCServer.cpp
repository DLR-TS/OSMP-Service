#include "GRPCServer.h"

void GRPCServer::startServer(const bool nonBlocking, const bool serverStopperActive)
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

	std::thread serverStopper;

	if (serverStopperActive) {
		serverStopper = std::thread([this]() {this->stopServer(); });
	}

	if (!nonBlocking) {
		server->Wait();
	}
	else {
		server_thread = std::make_unique<std::thread>(&grpc::Server::Wait, server);
	}
	if (serverStopperActive) {
		serverStopper.join();
	}
}

void GRPCServer::stopServer(const bool force)
{
	if (!force) {
		while (!serverStop.load(std::memory_order_relaxed)) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}
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
	serviceInterface->init(verbose, timeunit);

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
		if (mode == OSMPSERVICEMODE::PLAYBACK) { filename = port + CSVINPUTNAME; }
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

grpc::Status GRPCServer::GetOSIValue(grpc::ServerContext* context, const CoSiMa::rpc::String* request, CoSiMa::rpc::Bytes* response) {
	std::string message;
	int status = serviceInterface->readOSIMessage(request->value(), message);
	response->set_value(message);
	if (status == 0) {
		return grpc::Status::OK;
	} else {
		serverStop.store(true, std::memory_order_relaxed);
		return grpc::Status::CANCELLED;
	}
}

grpc::Status GRPCServer::GetStringValue(grpc::ServerContext* context, const CoSiMa::rpc::String* request, CoSiMa::rpc::String* response) {
	std::string value;
	serviceInterface->readParameter(request->value(), value);
	response->set_value(value);
	return grpc::Status::OK;
}

grpc::Status GRPCServer::SetOSIValue(grpc::ServerContext* context, const CoSiMa::rpc::NamedBytes* request, CoSiMa::rpc::Int32* response) {
	response->set_value(
		serviceInterface->writeOSIMessage(request->name(), request->value()));
	return grpc::Status::OK;
}

grpc::Status GRPCServer::SetStringValue(grpc::ServerContext* context, const CoSiMa::rpc::NamedString* request, CoSiMa::rpc::Int32* response) {
	serviceInterface->writeParameter(request->name(), request->value());
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
