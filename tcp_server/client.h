#ifndef CLIENT_H
#define CLIENT_H

/**
 * @author yurunsun@gmail.com
 */

#include "bizconnection.h"
//#include "handler.h"
//#include "safehandler.h"

#include <asio.hpp>
#include <map>
//#include <boost/timer.hpp>

class Client
    : public sigslot::has_slots<>
{
  protected:
    BizConnection::BizPtr m_pBizConnection;

  public:
    typedef void (Client::*RequestPtr)(BizPackage &);
    typedef std::map<uint32_t, RequestPtr> RequestMap;

    typedef void (Client::*NotifyPtr)(BizPackage &);
    typedef std::map<uint32_t, NotifyPtr> NotifyMap;

    explicit Client(const string &name = string(""));

    /// 继承类需要实现的提供外部的方法
    virtual void startServer() = 0;
    virtual bool sendToServer(YProto &proto) = 0;
    virtual void stopServer()
    {
        clearWaitforTimer();
        m_pBizConnection->stop();
    }

  protected:
    /// 继承类需要实现的初始化函数
    virtual void initRequestMap() { assert(false); }
    virtual void initNotifyMap() { assert(false); }
    virtual void initSignal();

    /// 继承类需要实现的钩子函数，用于处理网络事件
    virtual void onBizMsgArrived(core::Request &msg) = 0;
    virtual void onBizError(uint32_t ec, const string &em);
    virtual void onBizConnected() = 0;

    /// 继承类可以使用的工具方法
    /// 1. 心跳类
    void setKeepAliveSec(uint32_t sec) { m_keepAliveSec = sec; }
    uint32_t getKeepAliveSec() { return m_keepAliveSec; }
    void startKeepAlive();
    void keepAlive(const asio::error_code &e);
    virtual void onKeepAlive() { assert(false); }
    /// 2. 登陆状态类
    void setHasLogin(bool b) { m_hasLogin = b; }
    bool getHasLogin() { return m_hasLogin; }
    bool isOnline() { return (m_pBizConnection->isConnected() && m_hasLogin); }
    /// 3. 消息保存类
    template <typename Handler>
    bool savePendingCommand(Handler handler)
    {
        if (m_pendingCmd.size() < MaxPendingCommandCount)
        {
            m_pendingCmd.push_back(Command(handler));
            return true;
        }
        return false;
    }
    void sendPendingCommand()
    {
        if (isOnline())
        {
            vector<Command>::iterator it = m_pendingCmd.begin();
            for (; it != m_pendingCmd.end(); ++it)
            {
                (*it)();
            }
            m_pendingCmd.clear();
        }
    }
    /// 4. 延迟处理类
    typedef void (Client::*HoldonCallback)();
    void holdonSeconds(uint32_t sec, HoldonCallback func);
    void holdonHandler(HoldonCallback func, const asio::error_code &e);

    /// 5. waitfor 工具 处理异步消息超时
    typedef std::shared_ptr<asio::steady_timer> SharedTimerPtr;
    typedef std::unique_ptr<asio::steady_timer> ScopedTimerPtr;
    typedef std::shared_ptr<Probe> SharedProbe;
    typedef map<uint32_t, SharedTimerPtr> Uri2Timer;              /// 等待收到的包uri --> 这个时间timer
    void waitfor(uint32_t uri, uint32_t sec);                     /// 在发送req的时候调用，sec 秒数 uri 等待收到的uri
    void waitforTimeout(uint32_t uri, const asio::error_code &e); /// 所有waitfor超时都会自动回调这个函数
    virtual void onWaitforTimeout(uint32_t uri)
    {
        (void)uri;
        assert(false);
    }                                   /// 继承类覆盖这个钩子函数来进行错误处理
    void waitforReceived(uint32_t uri); /// 当响应函数handler被回调时，记得调用waitforReceived做清理工作
    void eraseWaitforTimer(uint32_t uri);
    void clearWaitforTimer();

    /// 继承类可以使用的工具成员：心跳 探针 请求阻塞
    typedef std::set<uint32_t> BlockReq;
    BlockReq m_block;
    SharedProbe m_probe;
    ScopedTimerPtr m_timer;
    uint32_t m_keepAliveSec;
    bool m_hasLogin;
    vector<Command> m_pendingCmd;
    static const uint32_t MaxPendingCommandCount = 20;
    ScopedTimerPtr m_holdonTimer;
    Uri2Timer m_uri2timer;
};

#define BIND_REQ(m, uri, callback) \
    m[static_cast<uint32_t>(uri)] = static_cast<RequestPtr>(callback);

#define BIND_NOTIFY(m, uri, callback) \
    m[static_cast<uint32_t>(uri)] = static_cast<NotifyPtr>(callback);

#endif // CLIENT_H