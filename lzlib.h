//
// Created by alex on 09/07/2015.
//

#ifndef LUAZLIB_LZLIB_H
#define LUAZLIB_LZLIB_H

#if defined(_MSC_VER)
    //  Microsoft
    #define LUA_LIBRARY __declspec(dllexport)
#else
    //  GCC
    #define LUA_LIBRARY __attribute__((visibility("default")))
#endif

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
    #include "zlib.h"

    LUA_LIBRARY void lua_lzlibopen(lua_State *L);
}

#endif //LUAZLIB_LZLIB_H
