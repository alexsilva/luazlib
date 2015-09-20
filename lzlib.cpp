//
// Created by alex on 09/07/2015.
//

#include <stddef.h>
#include <assert.h>
#include <string.h>
#include "lzlib.h"
#include <vector>
#include <stdio.h>

#define CHUNK 16384

/* report a zlib or i/o error */
zlib_state zlib_set_state(int ret) {
    zlib_state zlib_st;
    zlib_st.code = ret;
    switch (ret) {
        case Z_ERRNO:
            if (ferror(stdin))
                zlib_st.msg = "error reading stdin";
            if (ferror(stdout))
                zlib_st.msg = "error writing stdout";
            break;
        case Z_STREAM_ERROR:
            zlib_st.msg = "invalid compression level";
            break;
        case Z_DATA_ERROR:
            zlib_st.msg = "invalid or incomplete deflate data";
            break;
        case Z_MEM_ERROR:
            zlib_st.msg = "out of memory";
            break;
        case Z_VERSION_ERROR:
            zlib_st.msg = "zlib version mismatch!";
            break;
        default:
            zlib_st.msg = "ok";
    }
    return zlib_st;
}

int _compress(char *data, std::vector<unsigned char> &buff, int level) {
    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    ret = deflateInit(&strm, level);
    if (ret != Z_OK)
        return ret;

    /* compress until end of file */
    size_t remain = CHUNK;
    uInt chunk_size = 0;
    int data_size  = strlen(data);

    do {
        chunk_size = remain > data_size ? (size_t) data_size : remain;
        memcpy(in, data, chunk_size);
        strm.avail_in = chunk_size;
        data += chunk_size;

        flush = data[0] == '\0' ? Z_FINISH : Z_NO_FLUSH;

        data_size -= chunk_size;
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;

            ret = deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;

            if (have > 0) {
                buff.insert(buff.end(), out, out + have);
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */

    /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
    return Z_OK;
}

void static compress(lua_State *L) {
    char *data = luaL_check_string(L, 1);
    if (!data[strlen(data)] == '\0') {
        lua_error(L, (char *) "compress(zlib|eof): invalid string!");
    }
    lua_Object level_Object = lua_getparam(L, 2);
    int level = lua_isnumber(L, level_Object) ? (int) lua_getnumber(L, level_Object) : Z_DEFAULT_COMPRESSION;
    std::vector<unsigned char> buff;

    int ret = _compress(data, buff, level);
    zlib_state zlib_st = zlib_set_state(ret);

    if (zlib_st.code == Z_OK) {
        lua_pushnumber(L,  zlib_st.code);  // everything ok
        lua_pushlstring(L, (char *) buff.data(), (long) buff.size());
    } else {
        lua_pushnumber(L,  zlib_st.code);  // error
        lua_pushstring(L, (char *) zlib_st.msg.c_str());
    }
}

/* Decompress data */
static int _decompress(char *data, int data_size, std::vector<unsigned char> &buff) {
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;

    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

    /* decompress until deflate stream ends or end of file */
    size_t remain = CHUNK;
    uInt chunk_size = 0;

    do {
        chunk_size = remain > data_size ? (size_t) data_size : remain;

        memcpy(in, data, chunk_size);
        strm.avail_in = chunk_size;

        data += chunk_size;
        data_size -= chunk_size;

        if (strm.avail_in == 0)
            break;
        strm.next_in = in;
        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;

            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */

            switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;     /* and fall through */
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    (void)inflateEnd(&strm);
                    return ret;
                default:break;
            }
            have = CHUNK - strm.avail_out;
            if (have > 0) {
                buff.insert(buff.end(), out, out + have);
            }
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

void static decompress(lua_State *L) {
    std::vector<unsigned char> buff;
    lua_Object obj = lua_getparam(L, 1);
    if (!lua_isstring(L, obj)) {
        lua_error(L, (char *) "decompress(zlib): string required!");
    }
    int data_size = lua_strlen(L, obj);
    char *data = lua_getstring(L, obj);

    int ret = _decompress(data, data_size, buff);
    zlib_state zlib_st = zlib_set_state(ret);

    if (zlib_st.code == Z_OK) {
        lua_pushnumber(L,  zlib_st.code);  // everything ok
        lua_pushlstring(L, (char *) buff.data(), (long) buff.size());
    } else {
        lua_pushnumber(L,  zlib_st.code);  // error
        lua_pushstring(L, (char *) zlib_st.msg.c_str());
    }
}

static struct luaL_reg lzlib[] = {
    {(char *) "zlib_compress", compress},
    {(char *) "zlib_decompress", decompress}
};


LUA_LIBRARY void lua_lzlibopen(lua_State *L) {
    luaL_openlib(L, lzlib, (sizeof(lzlib) / sizeof(lzlib[0])));
    // global variables to configure using as parameter the
    // decompression function.
    lua_pushnumber(L, Z_NO_COMPRESSION);
    lua_setglobal(L, (char *) "Z_NO_COMPRESSION");

    lua_pushnumber(L, Z_BEST_SPEED);
    lua_setglobal(L, (char *) "Z_BEST_SPEED");

    lua_pushnumber(L, Z_BEST_COMPRESSION);
    lua_setglobal(L, (char *) "Z_BEST_COMPRESSION");

    lua_pushnumber(L, Z_DEFAULT_COMPRESSION);
    lua_setglobal(L, (char *) "Z_DEFAULT_COMPRESSION");
}