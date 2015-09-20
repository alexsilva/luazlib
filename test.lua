--
-- Created by IntelliJ IDEA.
-- User: alex
-- Date: 26/07/2015
-- Time: 20:22
-- To change this template use File | Settings | File Templates.
--

handle, msg = loadlib(getenv("LIBRARY_PATH"))

if (not handle or handle == -1) then error(msg) end

callfromlib(handle, 'lua_lzlibopen')

local text = ''
local stext = "THIS IS A TEST"
local i = 0
while i < 10000 do
    text = text..' '..stext
    i = i + 1
end
local original_text_size = strlen(text)

local code, data = zlib_compress(text, zlib.Z_BEST_COMPRESSION)

assert(code == 0, 'zlib_compress: invalid state code!')

local text_size_compressed = strlen(data)

local code, data = zlib_decompress(data)

assert(code == 0, 'zlib_decompress: invalid state code!')

local text_size_decompressed = strlen(data)

assert(data == text, '[equal] decompress error!')
assert(original_text_size == text_size_decompressed, '[text-size] decompressed error!')

print("Compress flags")
print("==============")
print("zlib.Z_NO_COMPRESSION:", zlib.Z_NO_COMPRESSION)
print("zlib.Z_BEST_COMPRESSION:", zlib.Z_BEST_COMPRESSION)
print("zlib.Z_DEFAULT_COMPRESSION:", zlib.Z_DEFAULT_COMPRESSION)
print("zlib.Z_BEST_SPEED:", zlib.Z_BEST_SPEED)
print("\n")

print("Results check")
print("==============")
print(
    tostring(original_text_size)..'b -',
    tostring(text_size_compressed)..'b =',
    tostring(original_text_size - text_size_compressed)..'b >>>',
    tostring(100.0 / original_text_size * text_size_compressed)..'%'
)

local code, data = zlib_compress("12345", zlib.Z_BEST_COMPRESSION + 100)
assert(code ~= 0)
print(code, data)