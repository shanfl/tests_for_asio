#include "bizconnection.h"
#include "tcpconnection.h"

std::set<uint32_t> BizConnection::m_needError;

BizConnection::BizConnection(asio::io_service &io_service, const string &name)
    : TcpConnection(io_service, name)
{
}

void BizConnection::sendBizMsg(unsigned uri, const BizPackage &pkg)
{
    /// TODO: usually this BizPackage contains the buffer of stream data to be sent to farpoint
    /// You should implement this by retriving buffer in BizPackage then call TcpConnection::send();
}

void BizConnection::onConnectSuccess()
{
    receiveHead();
    BizConnected.emit();
}

void BizConnection::onConnectFailure(const asio::error_code &e)
{
    if (e == asio::error::operation_aborted)
    {
        // INFO("%s %s %u operation aborted... %s", STR(getName()), STR(getip()), getport(), STR(e.message()));
    }
    else if ((e == asio::error::already_connected) || (e == asio::error::already_open) || (e == asio::error::already_started))
    {
        // WARN("%s %s %u alread connected... %s", STR(getName()), STR(getip()), getport(), STR(e.message()));
    }
    else
    {
        handleError("onConnectFailure", (int)eErrorType::S_FATAL, e);
    }
}

void BizConnection::onReceiveHeadSuccess(TcpConnection::DataBuffer &data)
{
    uint32_t pkglen = 0;
    if (peekLength(data.data(), data.size(), pkglen))
    {
        receiveBody(pkglen - getHeadLength());
    }
    else
    {
        handleError("peekLength", (int)eErrorType::S_FATAL);
    }
}

void BizConnection::onReceiveBodySuccess(TcpConnection::DataBuffer &data)
{
    /// This is simply an example, actually it's user's duty to unmarshal buffer to package.
    BizPackage msg;
    msg.unserializeFrom(data);
    BizMsgArrived.emit(msg);
    receiveHead();
}

void BizConnection::onReceiveFailure(const asio::error_code &e)
{
    if ((e == asio::error::operation_aborted))
    {
        //INFO("%s operation_aborted... %s", STR(getFarpointInfo()), STR(e.message()));
    }
    else
    {
        handleError("onReceiveFailure", (int)eErrorType::S_FATAL, e);
    }
}

void BizConnection::onSendSuccess()
{
    /// Leave it blank
}

void BizConnection::onSendFailure(const asio::error_code &e)
{
    if ((e == asio::error::operation_aborted))
    {
        //INFO("%s operation_aborted... %s", STR(getFarpointInfo()), STR(e.message()));
    }
    else
    {
        handleError("onSendFailure", (int)eErrorType::S_FATAL, e);
    }
}

void BizConnection::onTimeoutFailure(const asio::error_code &e)
{
    if ((e == asio::error::operation_aborted))
    {
        //INFO("%s operation_aborted... %s", STR(getFarpointInfo()), STR(e.message()));
    }
    else
    {
        //handleError("onTimeoutFailure", S_FATAL);
    }
}

void BizConnection::onCommonError(uint32_t ec, const string &em)
{
    handleError(em, ec);
}

void BizConnection::handleError(const string &from, uint32_t ec, const asio::error_code &e /* = asio::error_code()*/)
{
    stringstream ss;
    ss << getFarpointInfo() << " " << from;
    if (e)
    {
        ss << " asio " << e.value() << " " << e.message();
    }
    BizError.emit(ec, ss.str());
}