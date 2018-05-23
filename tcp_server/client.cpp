#include "client.h"

Client::Client(const string &name)
    : m_pBizConnection(BizConnection::create(ioService::instance(), name)), m_probe(new Probe), m_keepAliveSec(10), m_hasLogin(false)
{
}

void Client::initSignal()
{
    m_pBizConnection->BizError.connect(this, &Client::onBizError);
    m_pBizConnection->BizMsgArrived.connect(this, &Client::onBizMsgArrived);
    m_pBizConnection->BizConnected.connect(this, &Client::onBizConnected);
}

void Client::onBizError(uint32_t ec, const string &em)
{
    m_facade.serverError.emit(ec, em);
}

void Client::startKeepAlive()
{
    m_timer.reset(new asio::steady_timer(m_facade.io_service_ref));
    m_timer->expires_from_now(std::chrono::seconds(m_keepAliveSec));
    m_timer->async_wait(std::bind(&Client::keepAlive, this, m_probe));
}

void Client::keepAlive(const asio::error_code &e)
{
    if (e != asio::error::operation_aborted)
    {
        //FINE("%u send ping to %s %s:%u", m_facade.m_pInfo->uid, STR(m_pBizConnection->getName()), STR(m_pBizConnection->getip()), m_pBizConnection->getport());
        onKeepAlive();
        m_timer->expires_from_now(std::chrono::seconds(m_keepAliveSec));
        m_timer->async_wait(std::bind(&Client::keepAlive, this, m_probe));
    }
}

void Client::holdonSeconds(uint32_t sec, HoldonCallback func)
{
    m_holdonTimer.reset(new asio::steady_timer(m_facade.io_service_ref));
    m_holdonTimer->expires_from_now(std::chrono::seconds(sec));
    //SafeHandler1Bind1<Client, HoldonCallback, const asio::error_code&> h(&Client::holdonHandler, this, func, m_probe);
    m_holdonTimer->async_wait(std::bind(&Client::holdonHandler, this, func, m_probe));
}

void Client::holdonHandler(HoldonCallback func, const asio::error_code &e)
{
    if (!e)
    {
        if (m_holdonTimer != NULL)
            m_holdonTimer->cancel();
        (this->*func)();
    }
    else
    {
        //WARN("error: %s", STR(e.message()));
    }
}

void Client::waitfor(uint32_t uri, uint32_t sec)
{
    SharedTimerPtr t(new asio::steady_timer(m_facade.io_service_ref));
    t->expires_from_now(boost::posix_time::seconds(sec));
    t->async_wait(SafeHandler1Bind1<Client, uint32_t, const asio::error_code &>(
        &Client::waitforTimeout, this, uri, m_probe));
    m_uri2timer[uri] = t;
}

void Client::waitforTimeout(uint32_t uri, const asio::error_code &e)
{
    if (e != asio::error::operation_aborted)
    {
        //FATAL("%s waitfor uri %u timeout", STR(m_pBizConnection->getName()), uri);
        eraseWaitforTimer(uri);
        onWaitforTimeout(uri);
    }
}

void Client::waitforReceived(uint32_t uri)
{
    eraseWaitforTimer(uri);
}

void Client::eraseWaitforTimer(uint32_t uri)
{
    Uri2Timer::iterator it = m_uri2timer.find(uri);
    if (it != m_uri2timer.end())
    {
        SharedTimerPtr &t = it->second;
        if (t)
        {
            asio::error_code e;
            t->cancel(e);
            t.reset();
        }
        m_uri2timer.erase(it);
    }
}

void Client::clearWaitforTimer()
{
    Uri2Timer::iterator it = m_uri2timer.begin();
    for (; it != m_uri2timer.end(); ++it)
    {
        SharedTimerPtr &t = it->second;
        if (t)
        {
            asio::error_code e;
            t->cancel(e);
            t.reset();
        }
    }
    m_uri2timer.clear();
}