#include "GRPCInterface.h"

void GRPCInterface::startServer(const bool nonBlocking)
{
	if (server) {
		server->Shutdown(std::chrono::system_clock::now() + transaction_timeout);
	}
	grpc::ServerBuilder builder;
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(this);
	server = builder.BuildAndStart();
	std::cout << "Server listening on " << server_address << std::endl;
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

grpc::Status GRPCInterface::SetConfig(grpc::ServerContext* context, const CoSiMa_rpc::SimConfig* config, CoSiMa_rpc::SimInt32* response)
{
	int i_response = osmpInterface.create(config->fmu());
	i_response += osmpInterface.init();
	response->set_value(i_response);
	return grpc::Status::OK;
}

grpc::Status GRPCInterface::GetStringValue(grpc::ServerContext* context, const CoSiMa_rpc::SimString* request, CoSiMa_rpc::SimString* response) {
	response->set_value(
		osmpInterface.read(request->value()));
	return grpc::Status::OK;
};

grpc::Status GRPCInterface::SetStringValue(grpc::ServerContext* context, const CoSiMa_rpc::SimNamedString* request, CoSiMa_rpc::SimInt32* response) {
	response->set_value(
		osmpInterface.write(request->name(), request->value()));
	return grpc::Status::OK;
};

grpc::Status GRPCInterface::DoStep(grpc::ServerContext* context, const CoSiMa_rpc::SimEmpty* request, CoSiMa_rpc::SimDouble* response) {
	response->set_value(
		osmpInterface.doStep());//step size 1
	return grpc::Status::OK;
};
