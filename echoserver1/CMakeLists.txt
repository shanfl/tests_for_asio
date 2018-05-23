project(echoserver1)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../asio/asio/include)
add_executable(echoserver1 echoserver1.cpp)
add_executable(echoclient1 echoclient1.cpp)
