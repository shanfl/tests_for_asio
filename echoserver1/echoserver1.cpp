//  
// async_tcp_echo_server.cpp  
// ~~~~~~~~~~~~~~~~~~~~~~~~~  
//  
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)  
//  
// Distributed under the Boost Software License, Version 1.0. (See accompanying  
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)  
//  
#include <cstdlib>  
#include <iostream>  
#include "../common/the_asio_def.h"
using asio::ip::tcp;  
class session  
{  
public:  
  session(asio::io_service& io_service)  
    : socket_(io_service)  
  {  
  } 

  tcp::socket& socket()  
  {  
    return socket_;  
  }  

  void start()  
  {  
	socket_.async_read_some(asio::buffer(data_, max_length),std::bind(&session::handle_read, this,std::placeholders::_1, std::placeholders::_2));
  }

  void handle_read(const std::error_code& error,size_t bytes_transferred)  
  {  
    if (!error)  
    {  
      asio::async_write(socket_,asio::buffer(data_, bytes_transferred),bind(&session::handle_write, this,std::placeholders::_1));  
    }  
    else  
    {  
      delete this;  
    }  
  }  
  void handle_write(const asio::error_code& error)  
  {  
    if (!error)  
    {  
      socket_.async_read_some(asio::buffer(data_, max_length),bind(&session::handle_read, this,std::placeholders::_1,std::placeholders::_2));
    }  
    else  
    {  
      delete this;  
    }  
  }  
private:  
  tcp::socket socket_;  
  enum { max_length = 1024 };  
  char data_[max_length];  
};  
class server  
{  
public:  
  server(asio::io_service& io_service, short port): io_service_(io_service),acceptor_(io_service, tcp::endpoint(tcp::v4(), port))  
  {  
    session* new_session = new session(io_service_);  
    acceptor_.async_accept(new_session->socket(),bind(&server::handle_accept, this, new_session,std::placeholders::_1));  


  }  
  void handle_accept(session* new_session,  
      const std::error_code& error)  
  {  
    if (!error)  
    {  
      new_session->start();  
      new_session = new session(io_service_);  
      acceptor_.async_accept(new_session->socket(), bind(&server::handle_accept, this, new_session,std::placeholders::_1));  
    }  
    else  
    {  
      delete new_session;  
    }  
  }  
private:  
  asio::io_service& io_service_;  
  tcp::acceptor acceptor_;  
};  
int main(int argc, char* argv[])  
{  
  try  
  {  
    if (argc != 2)  
    {  
      std::cerr << "Usage: async_tcp_echo_server <port>/n";  
      return 1;  
    }  
    asio::io_service io_service;  
    using namespace std; // For atoi.  
    server s(io_service, atoi(argv[1]));  
    io_service.run();  
  }  
  catch (std::exception& e)  
  {  
    std::cerr << "Exception: " << e.what() << "/n";  
  }  
  return 0;  
} 