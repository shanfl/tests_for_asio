#ifndef _THE_ASIO_DEF_
#define _THE_ASIO_DEF_

#ifdef _WIN32 
#define _WIN32_WINNT 0x0501
#endif

#define ASIO_STANDALONE
//using error_code = std::error_code;
//using errc = std::errc;
#include "asio.hpp"
#endif