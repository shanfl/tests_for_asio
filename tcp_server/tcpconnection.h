#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

/**
 * @author yurunsun@gmail.com
 */

#include <asio.hpp>
#include <asio/deadline_timer.hpp>
//#include <boost/shared_ptr.hpp>
//#include <boost/enable_shared_from_this.hpp>
//#include <boost/timer.hpp>
#include <sstream>
#include <deque>
//#include "safehandler.h"
#include "def.h"
//#include "Probe.h"
typedef size_t Probe;
using namespace std;

class TcpConnection
    : public std::enable_shared_from_this<TcpConnection>
/*, private boost::noncopyable*/
{
  public:
    typedef std::vector<uint8_t> DataBuffer;
    typedef std::shared_ptr<TcpConnection> TcpPtr;
    static TcpPtr create(asio::io_service &io_service, const string &name) { return TcpPtr(new TcpConnection(io_service, name)); }
    virtual ~TcpConnection();
    void start(const string &ip, const string &port);
    void start(unsigned ip, uint16_t port);
    void start(const string &ip, uint16_t port);
    void stop();
    bool isConnected() { return m_socket.is_open(); }

    /// Getters and Setters
    void setName(const string &name) { m_name = name; }
    const string &getName() { return m_name; }
    void setHeadLength(uint32_t size) { m_headLength = size; }
    uint32_t getHeadLength() { return m_headLength; }
    void setConnectTimeoutSec(uint32_t sec) { m_connectTimeoutSec = sec; }
    uint32_t getConnectTimeoutSec() { return m_connectTimeoutSec; }
    const string &getip() { return m_ip; }
    uint16_t getport() { return m_port; }
    string getFarpointInfo()
    {
        stringstream ss;
        ss << m_name << " " << m_ip << ":" << m_port << " ";
        return ss.str();
    }

  protected:
    explicit TcpConnection(asio::io_service &io_service, const string &name);

    /// Provide for derived class
    void connect(asio::ip::tcp::endpoint endpoint);
    void receiveHead();
    void receiveBody(uint32_t bodyLength);
    void send(const void *data, uint32_t length);

    void TcpConnection::send(const std::vector<char> &vec);
    ///�ص�������֮ǰ��buffer�����У�ͬʱ����Ƿ��к����İ�
    ///void TcpConnection::handleSend(const asio::error_code &e);

    /// Class override callbacks
    virtual void onConnectSuccess() { assert(false); }
    virtual void onConnectFailure(const asio::error_code &e)
    {
        (void)e;
        assert(false);
    }
    virtual void onReceiveHeadSuccess(DataBuffer &data)
    {
        (void)data;
        assert(false);
    }
    virtual void onReceiveBodySuccess(DataBuffer &data)
    {
        (void)data;
        assert(false);
    }
    virtual void onReceiveFailure(const asio::error_code &e)
    {
        (void)e;
        assert(false);
    }
    virtual void onSendSuccess() { assert(false); }
    virtual void onSendFailure(const asio::error_code &e)
    {
        (void)e;
        assert(false);
    }
    virtual void onTimeoutFailure(const asio::error_code &e)
    {
        (void)e;
        assert(false);
    }
    virtual void onCommonError(uint32_t ec, const string &em)
    {
        (void)ec;
        (void)em;
        assert(false);
    }

  private:
    void checkDeadline(const asio::error_code &e);
    void handleConnect(const asio::error_code &e);
    void handleReceiveHead(const asio::error_code &e);
    void handleReceiveBody(const asio::error_code &e);
    void handleSend(const asio::error_code &e);

    typedef TcpConnection this_type;
    asio::ip::tcp::socket m_socket;
    asio::steady_timer m_deadline;
    //asio::basic_deadline_timer
    bool m_stopped;
    DataBuffer m_readBuf;
    string m_name;
    std::shared_ptr<Probe> m_probe;
    uint32_t m_headLength;
    uint32_t m_connectTimeoutSec;
    string m_ip;
    uint16_t m_port;

    deque<vector<char>> m_bufQueue;
};

#endif // TCPCONNECTION_H