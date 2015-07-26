cmake_minimum_required(VERSION 3.2)
project(luazlib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(
    ${LUA_DIR}/include
    ${ZLIB_DIR}
)

set(SOURCE_FILES lzlib.cpp lzlib.h)

add_library(lzlib SHARED ${SOURCE_FILES})
target_link_libraries(lzlib zlib ${LUA_LIB})
set_target_properties(lzlib PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")