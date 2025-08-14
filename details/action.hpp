#pragma once

#include <details/intdef.hpp>
#include <details/defs.hpp>

namespace fumen::details {

struct action {
    inner_operation m_operation;
    bool m_rise = false, m_mirror = false, m_colorize = false, m_comment = false, m_lock = false;
};

class action_codec {
public:
    action_codec(u32 _width, u32 _height, u32 _garbage_height)
    : m_width(_width), m_height(_height), m_garbage_height(_garbage_height) {}

private:
    u32 m_width, m_height, m_garbage_height;
    
    constexpr std::pair<u32, u32> m_decode_coordinate(
        i64 _value, piece _piece, rotation _rotation
    ) const {
        i32 _x = _value % m_width;
        i32 _y = m_height - (_value / 10) - 1;

        if (_piece == piece::O) {
            if (_rotation == rotation::spawn) return { _x, _y - 1 };
            else if (_rotation == rotation::reverse) return { _x + 1, _y };
            else if (_rotation == rotation::left) return { _x + 1, _y - 1 };
        } else if (_piece == piece::I) {
            if (_rotation == rotation::reverse) return { _x + 1, _y };
            else if (_rotation == rotation::left) return { _x, _y - 1 };
        } else if (_piece == piece::S) {
            if (_rotation == rotation::spawn) return { _x, _y - 1 };
            else if (_rotation == rotation::right) return { _x - 1, _y };
        } else if (_piece == piece::Z) {
            if (_rotation == rotation::spawn) return { _x, _y - 1 };
            else if (_rotation == rotation::left) return { _x + 1, _y };
        }

        return { _x, _y };
    }

    constexpr u64 _M_encode_coordinate(
        i32 _x, i32 _y, piece _piece, rotation _rotation
    ) const {
        if (!defs::is_mino(_piece)) _x = 0, _y = 22;
        else if (_piece == piece::O) {
            if (_rotation == rotation::spawn) _y++;
            else if (_rotation == rotation::reverse) _x--;
            else if (_rotation == rotation::left) _x--, _y++;
        } else if (_piece == piece::I) {
            if (_rotation == rotation::reverse) _x--;
            else if (_rotation == rotation::left) _y++;
        } else if (_piece == piece::S) {
            if (_rotation == rotation::spawn) _y++;
            else if (_rotation == rotation::right) _x++;
        } else if (_piece == piece::Z) {
            if (_rotation == rotation::spawn) _y++;
            else if (_rotation == rotation::left) _x--;
        }

        return (m_height - _y - 1) * m_width + _x;
    }

public:
    constexpr action decode(i64 _value) const {
        u32 _max_height = m_height + m_garbage_height,
            _block_count = m_width * _max_height;

        action _act;

        _act.m_operation.m_piece = static_cast<piece>(_value % 8);
        _value /= 8;

        _act.m_operation.m_rotation = static_cast<rotation>(_value % 4);
        _value /= 4;

        auto [_x, _y] = m_decode_coordinate(
            _value % _block_count,
            _act.m_operation.m_piece,
            _act.m_operation.m_rotation
        );
        _act.m_operation.m_x = _x;
        _act.m_operation.m_y = _y;
        _value /= _block_count;

        _act.m_rise = _value % 2;
        _value /= 2;

        _act.m_mirror = _value % 2;
        _value /= 2;

        _act.m_colorize = _value % 2;
        _value /= 2;

        _act.m_comment = _value % 2;
        _value /= 2;

        _act.m_lock = !(_value % 2);

        return _act;
    }

    constexpr i64 encode(action _act) const {
        u32 _max_height = m_height + m_garbage_height,
            _block_count = m_width * _max_height;

        i64 _value = 0;

        _value += !_act.m_lock;
        _value *= 2;
        _value += _act.m_comment;
        _value *= 2;
        _value += _act.m_colorize;
        _value *= 2;
        _value += _act.m_mirror;
        _value *= 2;
        _value += _act.m_rise;
        _value *= _block_count;
        _value += _M_encode_coordinate(
            _act.m_operation.m_x,
            _act.m_operation.m_y,
            _act.m_operation.m_piece,
            _act.m_operation.m_rotation
        );
        _value *= 4;
        _value += static_cast<u8>(_act.m_operation.m_rotation);
        _value *= 8;
        _value += static_cast<u8>(_act.m_operation.m_piece);

        return _value;
    }
};

}