project(xpd54_client)
include_directories(
  lib/include
)

add_executable(${PROJECT_NAME} simpleClient.cpp)