project(tcp_client)
add_definitions(-DASIO_STANDALONE)
add_definitions(-D_WIN32_WINNT=0x0501)
add_definitions(-DASIO_HAS_STD_TYPE_TRAITS)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../asio/asio/include)
add_executable(tcp_client tcp_client.cpp tcpconnection.h tcpconnection.cpp def.h Probe.h)
