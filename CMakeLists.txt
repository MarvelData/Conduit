cmake_minimum_required(VERSION 3.8)
project(RegBook)

set(CMAKE_CXX_STANDARD 17)
SET(EXECUTABLE_OUTPUT_PATH ../build)

add_executable(RegBook src/main.cpp src/Math.cpp src/Math.hpp src/StreamControl.cpp src/StreamControl.hpp src/Date.cpp src/Date.hpp src/PostInfo.cpp src/PostInfo.hpp src/Member.cpp src/Member.hpp src/Communication.cpp src/Communication.hpp src/Database.cpp src/Database.hpp src/Test.cpp src/Test.hpp)