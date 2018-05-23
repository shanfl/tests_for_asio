//
// main.cpp
// ~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "../common/the_asio_def.h"
#include <iostream>
#include <string>
#include <asio.hpp>
//#include <boost/bind.hpp>
//#include <boost/lexical_cast.hpp>
#include "server.hpp"

#define abc ASIO_NO_DEPRECATED

int main(int argc, char* argv[])
{
  try
  {
	  //int a = abc;
    // Check command line arguments.
    if (argc != 5)
    {
      std::cerr << "Usage: http_server <address> <port> <threads> <doc_root>\n";
      std::cerr << "  For IPv4, try:\n";
      std::cerr << "    receiver 0.0.0.0 80 1 .\n";
      std::cerr << "  For IPv6, try:\n";
      std::cerr << "    receiver 0::0 80 1 .\n";
      return 1;
    }

    // Initialise the server.
    std::size_t num_threads = atoi(argv[3]);
    http::server2::server s(argv[1], argv[2], argv[4], num_threads);

    // Run the server until stopped.
    s.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "exception: " << e.what() << "\n";
  }

  return 0;
}
