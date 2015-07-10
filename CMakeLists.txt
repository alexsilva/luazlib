cmake_minimum_required(VERSION 3.2)
project(luazlib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(
    ${LUA_DIR}/include
    ${LUA_DIR}/src
    ${ZLIB_PATH}
)

set(LIBRARIES
    ${LUA_DIR}/lib/mingw3/liblua.a
    ${LUA_DIR}/lib/mingw3/liblualib.a
    ${ZLIB_PATH}/Release/libzlib.dll
)

set(SOURCE_FILES
    lzlib.cpp
    lzlib.h
)

add_library(lzlib SHARED ${SOURCE_FILES})
target_link_libraries(lzlib ${LIBRARIES})