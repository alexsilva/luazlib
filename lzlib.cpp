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
void zerr(int ret)
{
    fputs("zpipe: ", stderr);
    switch (ret) {
        case Z_ERRNO:
            if (ferror(stdin))
                fputs("error reading stdin\n", stderr);
            if (ferror(stdout))
                fputs("error writing stdout\n", stderr);
            break;
        case Z_STREAM_ERROR:
            fputs("invalid compression level\n", stderr);
            break;
        case Z_DATA_ERROR:
            fputs("invalid or incomplete deflate data\n", stderr);
            break;
        case Z_MEM_ERROR:
            fputs("out of memory\n", stderr);
            break;
        case Z_VERSION_ERROR:
            fputs("zlib version mismatch!\n", stderr);
        default:break;
    }
}


void static compress(void) {

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

    buff.push_back('\0'); // end of stream

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

void static decompress(void) {
    std::vector<unsigned char> buff;
    lua_Object obj = lua_getparam(1);

    if (!lua_isstring(obj)) {
        lua_error((char *) "decompress: string required!");
    }
    int data_size = lua_strlen(obj);
    char *data = lua_getstring(obj);

    int ret = _decompress(data, data_size, buff);
    if (ret != Z_OK) zerr(ret);

    lua_pushstring((char *) buff.data());
}

static struct luaL_reg lzlib[] = {
    {(char *) "zlib_compress", compress},
    {(char *) "zlib_decompress", decompress}
};


LUA_LIBRARY void lua_lzlibopen(lua_State *L) {
    lua_state = L;
    luaL_openlib(lzlib, (sizeof(lzlib) / sizeof(lzlib[0])));
}