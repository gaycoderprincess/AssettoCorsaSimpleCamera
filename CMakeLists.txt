cmake_minimum_required(VERSION 3.27)
project(AssettoCorsaSimpleCamera)

SET(CMAKE_SYSTEM_NAME Windows)

SET(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
SET(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
SET(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)
SET(CMAKE_RANLIB x86_64-w64-mingw32-ranlib)

SET(CMAKE_CXX_STANDARD 20)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -masm=intel -s -static")

add_library(AssettoCorsaSimpleCamera SHARED main.cpp)
target_include_directories(AssettoCorsaSimpleCamera PUBLIC ../nya-common)
target_include_directories(AssettoCorsaSimpleCamera PUBLIC ../nya-common/3rdparty)
target_include_directories(AssettoCorsaSimpleCamera PUBLIC ../nya-common-ac)
target_link_options(AssettoCorsaSimpleCamera PRIVATE "-Wl,--exclude-all-symbols")

set_target_properties(AssettoCorsaSimpleCamera PROPERTIES PREFIX "")
set_target_properties(AssettoCorsaSimpleCamera PROPERTIES SUFFIX "_gcp.dll")