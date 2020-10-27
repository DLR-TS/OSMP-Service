#include "GRPCInterface.h"

void GRPCInterface::startServer(const bool nonBlocking)
{
	if (server)
		server->Shutdown(std::chrono::system_clock::now() + transaction_timeout);
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
	response->set_value(
		osmpInterface.create(config->fmu()));
	return grpc::Status::OK;
}
