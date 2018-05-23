#ifndef CROSSDOMAIN_H
#define CROSSDOMAIN_H

#include <string>
#include <asio.hpp>

/**
 * @author yurunsun@gmail.com
 */

class CrossDomain
{
  private:
    struct Server;
    std::shared_ptr<Server> m_pserver;
    CrossDomain(asio::io_service &io_service, const std::string &local_port);
    static CrossDomain *s_instance;

  public:
    static void create(asio::io_service &io_service, const std::string &local_port)
    {
        s_instance = new CrossDomain(io_service, local_port);
    }

    static CrossDomain *instance();
    void start_server();
};

#endif // CROSSDOMAIN_H