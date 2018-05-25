#include "def.h"
#include <asio.hpp>
#include "tcpconnection.h"


int main()
{

	asio::io_service ioservice;
	std::shared_ptr<TcpConnection> con = TcpConnection::create(ioservice, "tcp_client");
	con->start("127.0.0.0",9000);

	//ioservice.run();

	while (true)
	{
		ioservice.run_one();
	}
	return 0;
}