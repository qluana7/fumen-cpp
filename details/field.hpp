#pragma once

#include <string>

#include <details/intdef.hpp>

#include <details/defs.hpp>
#include <details/inner_field.hpp>

namespace fumen::details {

struct field_operation {
    piece_type m_piece;
    rotation_type m_rotation;
    u32 m_x, m_y;
};

struct mino {
    mino() = default;
    mino(piece_type _piece, rotation_type _rotation, u32 _x, u32 _y)
    : m_piece(_piece), m_rotation(_rotation), m_x(_x), m_y(_y) {}
    mino(const field_operation& _op)
    : m_piece(_op.m_piece), m_rotation(_op.m_rotation), m_x(_op.m_x), m_y(_op.m_y) {}

private:
    piece_type m_piece;
    rotation_type m_rotation;
    u32 m_x, m_y;

public:
    container_type positions() const {
        container_type _cont = field_util::get_block_positions(
            m_piece,
            m_rotation,
            m_x, m_y
        );

        std::sort(_cont.begin(), _cont.end(), [] (const auto& _a, const auto& _b) {
            if (_a.second == _b.second)
                return _a.first < _b.first;
            else
                return _a.second < _b.second;
        });

        return _cont;
    }

    bool is_valid() const {
        container_type _cont = positions();
        return std::all_of(_cont.begin(), _cont.end(), [] (const auto& _p) {
            auto [_x, _y] = _p;
            return 0 <= _x && _x < (i32)FIELD_WIDTH && 0 <= _y && _y < (i32)FIELD_HEIGHT;
        });
    }

    piece_type piece() const { return m_piece; }
    rotation_type rotation() const { return m_rotation; }
    u32 x() const { return m_x; }
    u32 y() const { return m_y; }

    explicit operator field_operation() const {
        return field_operation {
            m_piece,
            m_rotation,
            m_x,
            m_y
        };
    }
};

struct field {
    field() = default;
    field(const inner_field& _field) : m_field(_field) {}
    field(const std::string& _field) {
        m_field = inner_field(
            play_field::parse(_field),
            play_field(FIELD_WIDTH)
        );
    }
    field(const std::string& _field, const std::string& _garbage) {
        m_field = inner_field(
            play_field::parse(_field),
            play_field::parse(_garbage, FIELD_WIDTH)
        );
    }

private:
    inner_field m_field;

public:
    bool can_fill() const { return true; }
    bool can_fill(field_operation _op) const
    { return can_fill(mino(_op)); }
    bool can_fill(const mino& _mino) const
    { return m_field.can_fill_all(_mino.positions()); }

    bool can_lock() const { return true; }
    bool can_lock(field_operation _op) const
    { return can_lock(mino(_op)); }
    bool can_lock(const mino& _mino) const {
        if (!can_fill(_mino)) return false;

        field_operation _op = static_cast<field_operation>(_mino);
        _op.m_y -= 1;
        return !can_fill(_op);
    }

    void fill(field_operation _op, bool _force = false) { fill(mino(_op), _force); }
    void fill(const mino& _mino, bool _force = false) {
        if (!_force && !can_fill(_mino)) {
            throw std::invalid_argument("Cannot fill the field with the given mino.");
        }

        m_field.fill_all(_mino.positions(), _mino.piece());
    }

    void put(field_operation _op) { put(mino(_op)); }
    void put(const mino& _mino) {
        for (u32 _y = _mino.y(); _y >= 0; _y--) {
            if (!can_lock(_mino)) continue;

            fill(_mino);
        }
    }

    void clear_line() { m_field.clear_line(); }

    piece_type at(i32 _x, i32 _y) const
    { return m_field.get_number_at(_x, _y); }

    void set(u32 _x, u32 _y, piece_type _p)
    { m_field.set_number_at(_x, _y, _p); }

    std::string to_string(bool _reduced = true, char _sep = '\n', bool _garbage = true) const {
        i32 _min_y = _garbage ? -1 : 0;

        std::string _result;

        for (i32 _y = FIELD_HEIGHT - 1; _y >= _min_y; _y--) {
            std::string _line(FIELD_WIDTH, ' ');

            for (u32 _x = 0; _x < FIELD_WIDTH; _x++)
                _line[_x] = defs::to_char(at(_x, _y));
            
            if (_reduced && _line.find_first_not_of('_') == std::string::npos)
                continue;
            
            _reduced = false;

            _result += _line;

            if (_y != _min_y) _result += _sep;
        }

        return _result;
    }

    explicit operator inner_field() const {
        inner_field _field;

        for (i32 _y = -1; _y < (i32)FIELD_HEIGHT; _y++) {
            for (u32 _x = 0; _x < FIELD_WIDTH; _x++) {
                _field.set_number_at(_x, _y, at(_x, _y));
            }
        }

        return _field;
    }
};

}