//
// request_parser.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER2_REQUEST_PARSER_HPP
#define HTTP_SERVER2_REQUEST_PARSER_HPP

//#include <boost/logic/tribool.hpp>
//#include <boost/tuple/tuple.hpp>
#include "tribool.hpp"
#include <tuple>
namespace http {
namespace server2 {
	using tribool = xstd::neutribool;
struct request;

/// Parser for incoming requests.
class request_parser
{
public:
  /// Construct ready to parse the request method.
  request_parser();

  /// Reset to initial parser state.
  void reset();

  /// Result of parse.
  enum class result_type { good, bad, indeterminate };

  /// Parse some data. The tribool return value is true when a complete request
  /// has been parsed, false if the data is invalid, indeterminate when more
  /// data is required. The InputIterator return value indicates how much of the
  /// input has been consumed.
  template <typename InputIterator>
  std::tuple<result_type, InputIterator> parse(request& req,
      InputIterator begin, InputIterator end)
  {
    while (begin != end)
    {
	  result_type result = consume(req, *begin++);
	  if (result == result_type::good || result == result_type::bad)
        return std::make_tuple(result, begin);
    }
	result_type result = result_type::indeterminate;
    return std::make_tuple(result, begin);
  }

private:
  /// Handle the next character of input.
  result_type consume(request& req, char input);

  /// Check if a byte is an HTTP character.
  static bool is_char(int c);

  /// Check if a byte is an HTTP control character.
  static bool is_ctl(int c);

  /// Check if a byte is defined as an HTTP tspecial character.
  static bool is_tspecial(int c);

  /// Check if a byte is a digit.
  static bool is_digit(int c);

  /// The current state of the parser.
  enum state
  {
    method_start,
    method,
    uri,
    http_version_h,
    http_version_t_1,
    http_version_t_2,
    http_version_p,
    http_version_slash,
    http_version_major_start,
    http_version_major,
    http_version_minor_start,
    http_version_minor,
    expecting_newline_1,
    header_line_start,
    header_lws,
    header_name,
    space_before_header_value,
    header_value,
    expecting_newline_2,
    expecting_newline_3
  } state_;
};

} // namespace server2
} // namespace http

#endif // HTTP_SERVER2_REQUEST_PARSER_HPP
