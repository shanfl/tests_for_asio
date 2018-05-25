#include "crossdomain.h"
#include <string>
using std::string;

using asio::ip::tcp;
using std::uint8_t;
CrossDomain *CrossDomain::s_instance = NULL;

struct CrossDomainImpl : public std::enable_shared_from_this<CrossDomainImpl>
{
  public:
    static const unsigned MaxReadSize = 22;
    typedef std::shared_ptr<CrossDomainImpl> CrossDomainImplPtr;
    static CrossDomainImplPtr create(asio::io_service &io_service)
    {
        return CrossDomainImplPtr(new CrossDomainImpl(io_service));
    }

    tcp::socket &get_socket()
    {
        return m_socket;
    }

    void start()
    {
        start_read_some();
    }

    ~CrossDomainImpl()
    {
        close();
    }

    void close()
    {
        if (m_socket.is_open())
        {
            m_socket.close();
        }
    }

  private:
    CrossDomainImpl(asio::io_service &io_service)
        : m_socket(io_service)
    {
    }

    void start_read_some()
    {
		size_t size;
        m_socket.async_read_some(asio::buffer(m_readbuf, MaxReadSize),
                                 std::bind(&CrossDomainImpl::handle_read_some, shared_from_this(), std::placeholders::_1, std::placeholders::_2));

		//asio::async_read(m_socket, asio::buffer(m_readbuf, MaxReadSize), std::bind(&CrossDomainImpl::handle_read_some, this, std::placeholders::_1));
    }

    void handle_read_some(const asio::error_code &err,size_t size)
    {
        if (!err)
        {
            string str(m_readbuf);
            string reply("invalid");
            if (str == "<policy-file-request/>")
            {
                reply = "anything you wanna send back to client...";
            }
            asio::async_write(m_socket, asio::buffer(reply, reply.length()),
                              std::bind(&CrossDomainImpl::handle_write, shared_from_this(), std::placeholders::_1));
        }
    }

    void handle_write(const asio::error_code &error)
    {
        //FINE("CrossDomain handle_write, gonna close");
        close();
    }

    tcp::socket m_socket;
    char m_readbuf[MaxReadSize];
};

struct CrossDomain::Server
{
  private:
    CrossDomain *m_facade;
    tcp::acceptor m_acceptor;
    bool m_listened;
    string m_local_port;

  public:
    Server(asio::io_service &io_service, const string &local_port)
        : m_acceptor(io_service), m_listened(false), m_local_port(local_port)
    {
        // intend to leave it blank
    }
    ~Server()
    {
        if (m_acceptor.is_open())
        {
            //INFO("close server acceptor");
            m_acceptor.close();
        }
    }

    void start_server()
    {
        //FINE("CrossDomain start_server....");
        if (!m_listened)
        {
            //FINE("Try to listen...");
            try
            {
                tcp::endpoint ep(tcp::endpoint(tcp::v4(), atoi(m_local_port.c_str())));
				//tcp::endpoint ep(tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), atoi(m_local_port.c_str())));
                m_acceptor.open(ep.protocol());
                m_acceptor.bind(ep);
                m_acceptor.listen();
            }
            catch (const asio::system_error &ec)
            {
                //WARN("Port %s already in use! Fail to listen...", STR(m_local_port));
                return;
            }
            catch (...)
            {
                //WARN("Unknown error while trying to listen...");
                return;
            }
            m_listened = true;
            //FINE("Listen port %s succesfully!", STR(m_local_port));
        }

        CrossDomainImpl::CrossDomainImplPtr new_server_impl = CrossDomainImpl::create(m_acceptor.get_io_service());
        m_acceptor.async_accept(new_server_impl->get_socket(),
                                std::bind(&Server::handle_accept, this, new_server_impl, std::placeholders::_1));
    }

  private:
    void handle_accept(CrossDomainImpl::CrossDomainImplPtr pserver_impl, const asio::error_code &err)
    {
        //FINE("CrossDomain handle_accpet....");
        if (!err)
        {
            //FINE("CrossDomain everything ok, start...");
            pserver_impl->start(); // start this server
            start_server();        // waiting for another Tuna Connection
        }
        else
        {
            pserver_impl->close();
        }
    }
};

CrossDomain::CrossDomain(asio::io_service &io_service, const std::string &local_port)
    : m_pserver(new Server(io_service, local_port))
{
}

CrossDomain *CrossDomain::instance()
{
    if (!s_instance)
    {
        return NULL;
    }
    return s_instance;
}

void CrossDomain::start_server()
{
    m_pserver->start_server();
}