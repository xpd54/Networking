cmake_minimum_required(VERSION 3.26)
project(Network)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include_directories(
  external/asio/include
  /usr/local/include
  )
add_executable(${PROJECT_NAME} src/main.cpp)