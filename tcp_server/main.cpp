#include "crossdomain.h"

int main()
{
	asio::io_service ioservice;
	CrossDomain::create(ioservice,"9000");
	CrossDomain::instance()->start_server();
	ioservice.run();
	return 0;
}