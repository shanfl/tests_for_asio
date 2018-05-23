//
// io_service_pool.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "server.hpp"
#include <stdexcept>
//#include <boost/bind.hpp>
//#include <boost/shared_ptr.hpp>
#include <thread>
#include <iostream>

namespace http {
namespace server2 {

io_service_pool::io_service_pool(std::size_t pool_size)
  : next_io_service_(0)
{
  if (pool_size == 0)
    throw std::runtime_error("io_service_pool size is 0");

  // asio ���ڵİ汾�Ѿ�����Ҫ��������
  // Give all the io_services work to do so that their run() functions will not
  // exit until they are explicitly stopped.
  for (std::size_t i = 0; i < pool_size; ++i)
  {
    io_service_ptr io_service(new asio::io_context);
    //work_ptr work(new asio::io_context::work(*io_service));
    io_services_.push_back(io_service);
    //work_.push_back(work);
  }
}

void io_service_pool::run()
{
  // Create a pool of threads to run all of the io_services.
  std::vector<std::shared_ptr<std::thread> > threads;
  for (std::size_t i = 0; i < io_services_.size(); ++i)
  {
    std::shared_ptr<std::thread> thethread(new std::thread(static_cast<std::size_t(asio::io_context::*)(void)>(&asio::io_context::run), io_services_[i]));
    threads.push_back(thethread);
  }

  // Wait for all threads in the pool to exit.
  for (std::size_t i = 0; i < threads.size(); ++i)
    threads[i]->join();

  std::cout << "io_service_pool::run() end" << std::endl;
}

void io_service_pool::stop()
{
  // Explicitly stop all io_services.
  for (std::size_t i = 0; i < io_services_.size(); ++i)
    io_services_[i]->stop();
}

asio::io_context& io_service_pool::get_io_service()
{
  // Use a round-robin scheme to choose the next io_service to use.
  asio::io_context& io_service = *io_services_[next_io_service_];
  ++next_io_service_;
  if (next_io_service_ == io_services_.size())
    next_io_service_ = 0;
  return io_service;
}

} // namespace server2
} // namespace http
