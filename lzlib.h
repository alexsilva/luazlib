//
// Created by alex on 09/07/2015.
//

#ifndef LUAZLIB_LZLIB_H
#define LUAZLIB_LZLIB_H

#define LUA_LIBRARY __declspec(dllexport)

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
    #include "zlib.h"

    LUA_LIBRARY void lua_lzlibopen(lua_State *L);
}

#endif //LUAZLIB_LZLIB_H
