#include "io/io.h"
#include <memory>
#include "util/endian.h"
#include "util/range.h"

using namespace au;
using namespace au::io;

IO::~IO()
{
}

void IO::peek(size_t offset, std::function<void()> func)
{
    size_t old_pos = tell();
    seek(offset);
    try
    {
        func();
        seek(old_pos);
    }
    catch (...)
    {
        seek(old_pos);
        throw;
    }
}

bool IO::eof() const
{
    return tell() == size();
}

bstr IO::read(size_t bytes)
{
    if (!bytes)
        return ""_b;
    bstr ret(bytes);
    read_impl(&ret[0], bytes);
    return ret;
}

bstr IO::read_to_zero()
{
    bstr output;
    char c;
    while ((c = read_u8()) != '\0')
        output += c;
    return output;
}

bstr IO::read_to_zero(size_t bytes)
{
    bstr output = read(bytes);
    for (auto i : util::range(output.size()))
        if (!output[i])
            return output.substr(0, i);
    return output;
}

bstr IO::read_to_eof()
{
    return read(size() - tell());
}

bstr IO::read_line()
{
    bstr output;
    char c;
    while (!eof())
    {
        c = read_u8();
        if (c == '\0' || c == '\n')
            break;
        if (c != '\r')
            output += c;
    }
    return output;
}

u8 IO::read_u8()
{
    u8 ret = 0;
    read_impl(&ret, 1);
    return ret;
}

u16 IO::read_u16_le()
{
    u16 ret = 0;
    read_impl(&ret, 2);
    return util::from_little_endian<u16>(ret);
}

u32 IO::read_u32_le()
{
    u32 ret = 0;
    read_impl(&ret, 4);
    return util::from_little_endian<u32>(ret);
}

u64 IO::read_u64_le()
{
    u64 ret = 0;
    read_impl(&ret, 8);
    return util::from_little_endian<u64>(ret);
}

u16 IO::read_u16_be()
{
    u16 ret = 0;
    read_impl(&ret, 2);
    return util::from_big_endian<u16>(ret);
}

u32 IO::read_u32_be()
{
    u32 ret = 0;
    read_impl(&ret, 4);
    return util::from_big_endian<u32>(ret);
}

u64 IO::read_u64_be()
{
    u64 ret = 0;
    read_impl(&ret, 8);
    return util::from_big_endian<u64>(ret);
}

void IO::write(const bstr &bytes)
{
    if (!bytes.size())
        return;
    write_impl(bytes.get<char>(), bytes.size());
}

void IO::write_u8(u8 value)
{
    write_impl(&value, 1);
}

void IO::write_u16_le(u16 value)
{
    value = util::to_little_endian<u16>(value);
    write_impl(&value, 2);
}

void IO::write_u32_le(u32 value)
{
    value = util::to_little_endian<u32>(value);
    write_impl(&value, 4);
}

void IO::write_u64_le(u64 value)
{
    value = util::to_little_endian<u64>(value);
    write_impl(&value, 8);
}

void IO::write_u16_be(u16 value)
{
    value = util::to_big_endian<u16>(value);
    write_impl(&value, 2);
}

void IO::write_u32_be(u32 value)
{
    value = util::to_big_endian<u32>(value);
    write_impl(&value, 4);
}

void IO::write_u64_be(u64 value)
{
    value = util::to_big_endian<u64>(value);
    write_impl(&value, 8);
}
