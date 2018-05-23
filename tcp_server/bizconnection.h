#ifndef BIZCONNECTION_H
#define BIZCONNECTION_H

/**
 * @author yurunsun@gmail.com
 */

#include <asio.hpp>
#include <cstdio>
#include <stdexcept>
#include <string>
#include "sigslot/sigslot.h"

#include "tcpconnection.h"
#include "BizPackage.h"
using std::string;
class BizConnection
    : public TcpConnection
{
  public:
    typedef std::shared_ptr<BizConnection> BizPtr;
    static BizPtr create(asio::io_service &io_service, const string &name = string(""))
    {
        return BizPtr(new BizConnection(io_service, name));
    }
    void sendBizMsg(uint32_t uri, const BizPackage &pkg);

    sigslot::signal0<> BizConnected;
    sigslot::signal2<uint32_t, const string &> BizError;
    sigslot::signal0<> BizClosed;
    sigslot::signal1<BizPackage &> BizMsgArrived;

  protected:
    explicit BizConnection(asio::io_service &io_service, const string &name);

    /// Implement callbacks in base class
    virtual void onConnectSuccess();
    virtual void onConnectFailure(const asio::error_code &e);
    virtual void onReceiveHeadSuccess(DataBuffer &data);
    virtual void onReceiveBodySuccess(DataBuffer &data);
    virtual void onReceiveFailure(const asio::error_code &e);
    virtual void onSendSuccess();
    virtual void onSendFailure(const asio::error_code &e);
    virtual void onTimeoutFailure(const asio::error_code &e);
    virtual void onCommonError(uint32_t ec, const string &em);

    static void initNeedErrorSet();
    static std::set<uint32_t> m_needError;

  private:
    inline bool peekLength(void *data, uint32_t length, uint32_t &outputi32);
    void handleError(const string &from, uint32_t ec, const asio::error_code &e = asio::error_code());
};

inline bool BizConnection::peekLength(void *data, uint32_t length, uint32_t &outputi32)
{
    if (length >= 4)
    {
        memcpy(&outputi32, data, sizeof(uint32_t));
        return true;
    }
    else
    {
        return false;
    }
}

#endif // BIZCONNECTION_H