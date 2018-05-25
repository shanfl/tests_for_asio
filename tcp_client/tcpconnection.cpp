#include "tcpconnection.h"

using asio::ip::tcp;

TcpConnection::TcpConnection(asio::io_service &io_service, const string &name)
	: m_socket(io_service), m_deadline(io_service), m_stopped(false), m_name(name), m_probe(new Probe), m_headLength(10), m_connectTimeoutSec(5), m_ip(""), m_port(0)
{
}

TcpConnection::~TcpConnection()
{
	stop();
}

void TcpConnection::start(const string &ip, const string &port)
{
	start(ip, atoi(port.c_str()));
}

void TcpConnection::start(unsigned ip, uint16_t port)
{
	connect(tcp::endpoint(asio::ip::address_v4(ip), port));
}

void TcpConnection::start(const string &ip, uint16_t port)
{
	fprintf(stderr, "ip = %s,port = %d\n", ip.c_str(), port);
	asio::ip::address_v4 addr_v4 = asio::ip::address_v4::from_string(ip);
	connect(tcp::endpoint(addr_v4, port));
}

void TcpConnection::stop()
{
	if (!m_stopped)
	{
		m_stopped = true;
		try
		{
			m_readBuf.clear();
			asio::error_code ignored;
			m_socket.shutdown(tcp::socket::shutdown_both, ignored);
			m_socket.close(ignored);
			m_deadline.cancel();
		}
		catch (const asio::system_error &err)
		{
			//FATAL("asio::system_error em %s", err.what());
		}
	}
}

void TcpConnection::connect(tcp::endpoint endpoint)
{
	m_stopped = false;
	m_ip = endpoint.address().to_string();
	m_port = endpoint.port();
	//INFO("Trying connect %s:%u ...%s", STR(m_ip), m_port, STR(m_name));

	m_deadline.expires_from_now(std::chrono::seconds(m_connectTimeoutSec));
	m_socket.async_connect(endpoint, std::bind(&TcpConnection::handleConnect, shared_from_this(), std::placeholders::_1));
	m_deadline.async_wait(std::bind(&TcpConnection::checkDeadline, this, std::placeholders::_1));

	//m_socket.is_open()
}

void TcpConnection::receiveHead()
{
	m_readBuf.resize(m_headLength);
	asio::async_read(m_socket, asio::buffer(&m_readBuf[0], m_headLength),
		std::bind(&TcpConnection::handleReceiveHead, shared_from_this(), std::placeholders::_1));
}

void TcpConnection::receiveBody(uint32_t bodyLength)
{
	if (!m_stopped)
	{
		if ((bodyLength <= MAX_BUFFER_SIZE) && (bodyLength > 0))
		{
			m_readBuf.resize(bodyLength + m_headLength);
			asio::async_read(m_socket, asio::buffer(&m_readBuf[m_headLength], bodyLength),
				std::bind(&TcpConnection::handleReceiveBody, shared_from_this(), std::placeholders::_1));
		}
		else
		{
			onCommonError((int)eErrorType::S_FATAL, "illegal bodyLength to call receiveBody");
		}
	}
	else
	{
		onCommonError((int)eErrorType::S_ERROR, "illegal to call receiveBody while tcp is not connected");
	}
}

#if 0
void TcpConnection::send(const void* data, uint32_t length)
{
	if (!m_stopped) {
		if (length <= MAX_BUFFER_SIZE) {
			asio::async_write(m_socket, asio::const_buffers_1(data, length),
				std::bind(&TcpConnection::handleSend, shared_from_this(), std::placeholders::_1));
		}
		else {
			onCommonError((int)eErrorType::S_ERROR, "too big length to call send");
		}
	}
	else {
		onCommonError((int)eErrorType::S_ERROR, "illegal to call send while tcp is not connected");
	}
}
#endif

///发送时入队列
void TcpConnection::send(const void *data, uint32_t length)
{
	if (!m_stopped)
	{
		if (length <= MAX_BUFFER_SIZE)
		{
			const char *begin = (const char *)data;
			vector<char> vec(begin, begin + length);

			bool isLastComplete = m_bufQueue.empty();
			m_bufQueue.push_back(vec);
			/// 如果没有残余的包，就直接发送
			if (isLastComplete)
			{
				vector<char> &b(m_bufQueue.front());
				send(b);
			}
		}
		else
		{
			onCommonError((int)eErrorType::S_ERROR, "too big length to call send");
		}
	}
	else
	{
		onCommonError((int)eErrorType::S_ERROR, "illegal to call send while tcp is not connected");
	}
}

void TcpConnection::send(const std::vector<char> &vec)
{
	if (!m_stopped)
	{
		asio::async_write(m_socket, asio::buffer(&vec[0], vec.size()), asio::transfer_all(),
			std::bind(&TcpConnection::handleSend, shared_from_this(), std::placeholders::_1));
	}
	else
	{
		onCommonError((int)eErrorType::S_ERROR, "illegal to call send while tcp is not connected");
	}
}

///回调函数将之前的buffer出队列，同时检查是否有后来的包
void TcpConnection::handleSend(const asio::error_code &e)
{
	if (!m_stopped)
	{
		if (!e)
		{
			m_bufQueue.pop_front();
			if (!m_bufQueue.empty())
			{
				std::vector<char> &b(m_bufQueue.front());
				send(b);
			}
			//onSendSuccess();
		}
		else if (isConnected())
		{
			onSendFailure(e);
		}
	}
	else
	{
		//INFO("%s %s %u user's canceled by stop()", STR(m_name), STR(m_ip), m_port);
	}
}

/**
 * @brief TcpConnection::checkDeadline
 * @param e
 * case1: m_stopped == true which means user canceled
 * case2: m_deadline.expires_at() <= asio::deadline_timer::traits_type::now()
 *          Check whether the deadline has passed. We compare the deadline against
			the current time since a new asynchronous operation may have moved the
			deadline before this actor had a chance to run.
 */
void TcpConnection::checkDeadline(const asio::error_code &e)
{
	fprintf(stderr, "checkDeadline e = %d\n", e.value());
	if (!m_stopped)
	{
		if (m_deadline.expires_at() <= std::chrono::steady_clock::now()) //asio::steady_timer::traits_type::now())
		{
			onTimeoutFailure(e);
			if (m_socket.is_open())
			{
				fprintf(stderr, "socket is open\n");
			}
			else
			{
				fprintf(stderr, "socket is not open\n");
			}
			fprintf(stderr, "now close socket\n");
			m_socket.close();
			//fprintf(stderr, "timeout! \n");
			//this->start("127.0.0.1", m_port);
		}
	}
}

void TcpConnection::handleConnect(const asio::error_code &e)
{
	if (!m_stopped)
	{
		if (!e)
		{
			m_deadline.cancel();
			onConnectSuccess();
		}
		else
		{
			onConnectFailure(e);

		}
	}
	else
	{
		//INFO("%s %s %u user's canceled by stop()", STR(m_name), STR(m_ip), m_port);
	}
}

void TcpConnection::handleReceiveHead(const asio::error_code &e)
{
	if (!m_stopped)
	{
		if (!e)
		{
			onReceiveHeadSuccess(m_readBuf);
		}
		else if (isConnected())
		{
			onReceiveFailure(e);
		}
	}
	else
	{
		//INFO("%s %s %u user's canceled by stop()", STR(m_name), STR(m_ip), m_port);
	}
}

void TcpConnection::handleReceiveBody(const asio::error_code &e)
{
	if (!m_stopped)
	{
		if (!e)
		{
			onReceiveBodySuccess(m_readBuf);
		}
		else if (isConnected())
		{
			onReceiveFailure(e);
		}
	}
	else
	{
		//INFO("%s %s %u user's canceled by stop()", STR(m_name), STR(m_ip), m_port);
	}
}

#if 0
void TcpConnection::handleSend(const asio::error_code &e)
{
	if (!m_stopped) {
		if (!e) {
			//onSendSuccess();
		}
		else if (isConnected()) {
			onSendFailure(e);
		}
	}
	else {
		//INFO("%s %s %u user's canceled by stop()", STR(m_name), STR(m_ip), m_port);
	}
}
#endif