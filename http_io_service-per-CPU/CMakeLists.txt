project(http_ioservice_per_cpu)
add_definitions(-DASIO_STANDALONE)
add_definitions(-D_WIN32_WINNT=0x0501)
add_definitions(-DASIO_HAS_STD_TYPE_TRAITS)
#add_definitions(-DASIO_NO_DEPRECATED)
# add_definitions(-DASIO_NO_DEPRECATED) ASIO_HAS_STD_TYPE_TRAITS

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../asio/asio/include)
file(GLOB HTTP_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(http_ioservice_per_cpu ${HTTP_SRC})