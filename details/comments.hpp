#pragma once

#include <string>
#include <string_view>

#include <details/intdef.hpp>
#include <details/math.hpp>

namespace fumen::details {

class comment_codec {
private:
    static constexpr std::string_view s_table =
        R"( !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~)";

    static constexpr u32 s_size = s_table.size() + 1;

public:
    static std::string decode(i64 _value) {
        std::string _str(4, ' ');

        for (u32 _i = 0; _i < 4; _i++) {
            _str[_i] = s_table[_value % s_size];
            _value /= s_size;
        }

        return _str;
    }

    static constexpr i64 encode(char _ch, u32 _cnt)
    { return (_ch - 32u) * math::powi<u64>(s_size, _cnt); }
};

}