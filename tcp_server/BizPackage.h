#ifndef BizPackage_H_
#define BizPackage_H_
#include "tcpconnection.h"
class BizPackage
{
public:
	//TcpConnection::DataBuffer
	bool unserializeFrom(TcpConnection::DataBuffer &)
	{
		return false;
	}
};
#endif