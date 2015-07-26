--
-- Created by IntelliJ IDEA.
-- User: alex
-- Date: 26/07/2015
-- Time: 20:22
-- To change this template use File | Settings | File Templates.
--

handle, msg = loadlib(getenv("LIBRARY_PATH"))

if (not handle or handle == -1) then
    error(msg)
end

callfromlib(handle, 'lua_lzlibopen')

local data = zlib_compress("THIS IS A TEST")
print(data)

local data = zlib_decompress(data)
print(data)