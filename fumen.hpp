#pragma once

#include <vector>
#include <string>

#include <optional>

#include <details/intdef.hpp>
#include <details/encoder.hpp>
#include <details/decoder.hpp>

namespace fumen {

using field = fumen::details::field;
using piece = fumen::details::piece;
using rotation = fumen::details::rotation;

struct operation {
    piece m_piece;
    rotation m_rotation;
    u32 m_x, m_y;
};

struct fumen_page {
    field m_field;
    std::string m_comment;
    std::optional<operation> m_operation;
    union flags {
        struct {
            u8 lock_bit : 1;
            u8 mirror_bit : 1;
            u8 colorize_bit : 1;
            u8 rise_bit : 1;
            u8 reserved : 4;
        };
        u8 all = 0;
    } m_flags;
};

using fumen_pages = std::vector<fumen_page>;

inline static std::string encode(const fumen_pages& _pgs) {
    fumen::details::encode_pages _epgs;
    _epgs.reserve(_pgs.size());

    for (const auto& _pg : _pgs) {
        fumen::details::encode_page _epg;

        _epg.m_field = _pg.m_field;
        _epg.m_comment = _pg.m_comment;
        if (_pg.m_operation)
            _epg.m_operation = fumen::details::field_operation {
                fumen::details::defs::to_char(_pg.m_operation->m_piece),
                fumen::details::defs::to_string(_pg.m_operation->m_rotation),
                _pg.m_operation->m_x,
                _pg.m_operation->m_y
            };
        _epg.m_flags.all = _pg.m_flags.all;

        _epgs.push_back(std::move(_epg));
    }

    return "v115@" + fumen::details::encoder::encode(_epgs);
}

inline static fumen_pages decode(const std::string& _str) {
    fumen::details::pages _pgs = fumen::details::decoder::decode(_str);

    fumen_pages _fpgs; _fpgs.reserve(_pgs.size());

    for (const fumen::details::page& _pg : _pgs) {
        fumen_page _fpg;

        _fpg.m_field = _pg.m_inner_field;
        _fpg.m_comment = _pg.m_comment.value_or("");
        if (_pg.m_operation)
            _fpg.m_operation = {
                fumen::details::defs::to_piece(_pg.m_operation->m_piece),
                fumen::details::defs::to_rotation(_pg.m_operation->m_rotation),
                _pg.m_operation->m_x,
                _pg.m_operation->m_y
            };
        _fpg.m_flags.all = _pg.m_flags.all;

        _fpgs.push_back(std::move(_fpg));
    }

    return _fpgs;
}

}