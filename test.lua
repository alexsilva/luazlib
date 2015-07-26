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

local data = zlib_compress(text)
local text_size_compressed = strlen(data)

local data = zlib_decompress(data)
local text_size_decompressed = strlen(data)

assert(data == text, '[equal] decompress error!')
assert(original_text_size == text_size_decompressed, '[text-size] decompressed error!')

print(data)
print(
    tostring(original_text_size)..'b -',
    tostring(text_size_compressed)..'b =',
    tostring(original_text_size - text_size_compressed)..'b >>>',
    tostring(100.0 / original_text_size * text_size_compressed)..'%'
)