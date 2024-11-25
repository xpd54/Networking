project(xpd54_server)
include_directories(
  lib/include
)

add_executable(${PROJECT_NAME} simpleServer.cpp)