#pragma once

#include <array>
#include <vector>

#include <algorithm>
#include <stdexcept>

#include <details/intdef.hpp>

#include <details/defs.hpp>

namespace fumen::details {

inline constexpr u32 GARBAGE_LINE = 1u;
inline constexpr u32 FIELD_WIDTH = 10;
inline constexpr u32 FIELD_HEIGHT = 23;
inline constexpr u32 PLAY_BLOCKS = FIELD_WIDTH * FIELD_HEIGHT;

using pos_type = std::pair<i32, i32>;
using container_type = std::array<pos_type, 4>;

/* static */ class field_util {
public:
    static constexpr container_type get_block_positions(piece _piece, rotation _rotation, u32 _x, u32 _y) {
        container_type _cont = get_blocks(_piece, _rotation);

        for (auto& [_cx, _cy] : _cont) {
            _cx += _x;
            _cy += _y;
        }

        return _cont;
    }

    static constexpr container_type get_blocks(piece _piece, rotation _rotation) {
        container_type _cont = get_pieces(_piece);

        switch (_rotation) {
            case rotation::spawn: return _cont;
            case rotation::right: return rotate_right(_cont);
            case rotation::reverse: return rotate_reverse(_cont);
            case rotation::left: return rotate_left(_cont);
        }

        throw std::invalid_argument("Invalid rotation");
    }

    static constexpr container_type get_pieces(piece _piece) {
        switch (_piece) {
            case piece::I: return { { { 0, 0 }, { -1, 0 }, { 1, 0 }, { 2, 0 } } };
            case piece::T: return { { { 0, 0 }, { -1, 0 }, { 1, 0 }, { 0, 1 } } };
            case piece::O: return { { { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 } } };
            case piece::L: return { { { 0, 0 }, { -1, 0 }, { 1, 0 }, { 1, 1 } } };
            case piece::J: return { { { 0, 0 }, { -1, 0 }, { 1, 0 }, { -1, 1 } } };
            case piece::S: return { { { 0, 0 }, { -1, 0 }, { 0, 1 }, { 1, 1 } } };
            case piece::Z: return { { { 0, 0 }, { 1, 0 }, { 0, 1 }, { -1, 1 } } };
            default: break;
        }

        throw std::invalid_argument("Invalid piece");
    }

    static constexpr container_type rotate_right(const container_type& _pieces) {
        container_type _result;
        for (u32 _i = 0; _i < 4; ++_i) {
            _result[_i] = { _pieces[_i].second, -_pieces[_i].first };
        }
        return _result;
    }
    
    static constexpr container_type rotate_left(const container_type& _pieces) {
        container_type _result;
        for (u32 _i = 0; _i < 4; ++_i) {
            _result[_i] = { -_pieces[_i].second, _pieces[_i].first };
        }
        return _result;
    }

    static constexpr container_type rotate_reverse(const container_type& _pieces) {
        container_type _result;
        for (u32 _i = 0; _i < 4; ++_i) {
            _result[_i] = { -_pieces[_i].first, -_pieces[_i].second };
        }
        return _result;
    }
};

struct play_field {
    play_field(const std::vector<piece>& _pieces, u32 _size = PLAY_BLOCKS)
    : m_pieces(_pieces), m_size(_size) {}
    play_field(u32 _size = PLAY_BLOCKS)
    : play_field(std::vector<piece>(_size, piece::empty), _size) {}

private:
    std::vector<piece> m_pieces;
    u32 m_size = 0;

public:
#if __cplusplus >= 202002L
    constexpr
#endif
    piece get(i32 _x, i32 _y) const
    { return m_pieces[_x + _y * FIELD_WIDTH]; }

    void set(i32 _x, i32 _y, piece _piece)
    { set_at(_x + _y * FIELD_WIDTH, _piece); }

    void set_at(u32 _idx, piece _piece)
    { m_pieces[_idx] = _piece; }

    void add_offset(i32 _x, i32 _y, i8 _value) {
        piece& _piece = m_pieces[_x + _y * FIELD_WIDTH];

        _piece = static_cast<piece>(
            static_cast<i8>(_piece) + static_cast<i8>(_value)
        );
    }

    void fill(inner_operation _op) {
        container_type _blocks = field_util::get_blocks(_op.m_piece, _op.m_rotation);

        for (const auto& [_bx, _by] : _blocks)
            set( _bx + _op.m_x, _by + _op.m_y, _op.m_piece);
    }

    void fill_all(const container_type& _cont, piece _piece) {
        for (const auto& [_x, _y] : _cont) {
            set(_x, _y, _piece);
        }
    }

    void clear_line() {
        std::vector<piece> _field = m_pieces;
        u32 _top = m_pieces.size() / FIELD_WIDTH - 1;

        for (i32 _y = _top; _y >= 0; _y--) {
            std::vector<piece> _line(
                _field.begin() + (_y * FIELD_WIDTH),
                _field.begin() + ((_y + 1) * FIELD_WIDTH)
            );

            if (std::all_of(_line.begin(), _line.end(), [](piece _piece) { return _piece != piece::empty; })) {
                std::vector<piece> _bottom(
                    _field.begin(), _field.begin() + (_y * FIELD_WIDTH)
                );

                _bottom.insert(
                    _bottom.end(),
                    _field.begin() + ((_y + 1) * FIELD_WIDTH),
                    _field.end()
                );

                _bottom.insert(_bottom.end(), FIELD_WIDTH, piece::empty);

                _field = std::move(_bottom);
            }
        }

        m_pieces = std::move(_field);
    }

    void up(const play_field& _up_field) {
        std::vector<piece> _piece = _up_field.m_pieces;
        _piece.insert(_piece.end(), m_pieces.begin(), m_pieces.end());

        _piece.resize(m_size);

        m_pieces = std::move(_piece);
    }

    void mirror() {
        for (u32 _y = 0; _y < m_size / FIELD_WIDTH; _y++)
            std::reverse(
                m_pieces.begin() + (_y * FIELD_WIDTH),
                m_pieces.begin() + ((_y + 1) * FIELD_WIDTH)
            );
    }

    void lshift() {
        u32 _height = m_size / FIELD_WIDTH;

        for (u32 _y = 0; _y < _height; _y++) {
            for (u32 _x = 0; _x < FIELD_WIDTH - 1; _x++)
                m_pieces[_x + _y * FIELD_WIDTH] =
                    m_pieces[_x + 1 + _y * FIELD_WIDTH];
                
            m_pieces[FIELD_WIDTH - 1 + _y * FIELD_WIDTH] = piece::empty;
        }
    }

    void rshift() {
        u32 _height = m_size / FIELD_WIDTH;

        for (u32 _y = 0; _y < _height; _y++) {
            for (u32 _x = FIELD_WIDTH - 1; _x > 0; _x--)
                m_pieces[_x + _y * FIELD_WIDTH] =
                    m_pieces[_x - 1 + _y * FIELD_WIDTH];
                
            m_pieces[_y * FIELD_WIDTH] = piece::empty;
        }
    }

    void up_shift() {
        std::vector<piece> _blocks(FIELD_WIDTH, piece::empty);
        _blocks.insert(_blocks.end(), m_pieces.begin(), m_pieces.end() - FIELD_WIDTH);

        m_pieces = std::move(_blocks);
    }

    void down_shift() {
        std::vector<piece> _blocks(m_pieces.begin() + FIELD_WIDTH, m_pieces.end());
        _blocks.insert(_blocks.begin(), FIELD_WIDTH, piece::empty);

        m_pieces = std::move(_blocks);
    }

    void clear() { m_pieces.assign(m_size, piece::empty); }

    const std::vector<piece>& get_pieces() const { return m_pieces; }
    u32 size() const { return m_size; }

    static play_field parse(const std::string& _lines, u32 _len = 0) {
        u32 _size = _len == 0 ? _lines.size() : _len;

        if (_size % FIELD_WIDTH != 0)
            throw std::invalid_argument("Invalid field length");
        
        play_field _field = _len == 0 ? play_field{} : play_field(_size);

        for (u32 _i = 0; _i < _size; _i++) {
            _field.set(
                _i % FIELD_WIDTH,
                (_size - _i - 1) / FIELD_WIDTH,
                defs::to_piece(_lines[_i])
            );
        }

        return _field;
    }
};

struct inner_field {
    inner_field(
        const play_field& _field = play_field(PLAY_BLOCKS),
        const play_field& _garbage = play_field(FIELD_WIDTH)
    ) : m_field(_field), m_garbage(_garbage) {}

private:
    play_field m_field, m_garbage;

public:
    void fill(inner_operation _op)
    { m_field.fill(_op); }

    void fill_all(const container_type& _cont, piece _piece)
    { m_field.fill_all(_cont, _piece); }

#if __cplusplus >= 202002L
    constexpr
#endif
    bool can_fill(piece _piece, rotation _rotation, i32 _x, i32 _y) const {
        container_type _pos = field_util::get_block_positions(_piece, _rotation, _x, _y);

        return std::all_of(_pos.begin(), _pos.end(), [this](const auto& _piece) {
            auto [_px, _py] = _piece;

            return 0 <= _px && _px < (i32)FIELD_WIDTH
                && 0 <= _py && _py < (i32)FIELD_HEIGHT
                && get_number_at(_px, _py) == piece::empty;
        });
    }

#if __cplusplus >= 202002L
    constexpr
#endif
    bool can_fill_all(const container_type& _cont) const {
        return std::all_of(_cont.begin(), _cont.end(), [this](const auto& _piece) {
            auto [_px, _py] = _piece;

            return 0 <= _px && _px < (i32)FIELD_WIDTH
                && 0 <= _py && _py < (i32)FIELD_HEIGHT
                && get_number_at(_px, _py) == piece::empty;
        });
    }

#if __cplusplus >= 202002L
    constexpr
#endif
    bool is_on_ground(piece _piece, rotation _rotation, i32 _x, i32 _y) const {
        return !can_fill(_piece, _rotation, _x, _y - 1);
    }

    void clear_line() { m_field.clear_line(); }

    void rise_garbage() {
        m_field.up(m_garbage);
        m_garbage.clear();
    }

    void mirror() { m_field.mirror(); }

    void lshift() { m_field.lshift(); }

    void rshift() { m_field.rshift(); }

    void up_shift() { m_field.up_shift(); }

    void down_shift() { m_field.down_shift(); }

    void add_number(i32 _x, i32 _y, i8 _value) {
        if (_y >= 0)
            m_field.add_offset(_x, _y, _value);
        else
            m_garbage.add_offset(_x, -(_y + 1), _value);
    }

    void set_number_field_at(u32 _idx, piece _piece)
    { m_field.set_at(_idx, _piece); }

    void set_number_garbage_at(u32 _idx, piece _piece)
    { m_garbage.set_at(_idx, _piece); }

    void set_number_at(i32 _x, i32 _y, piece _piece) {
        return _y >= 0 ?
            m_field.set(_x, _y, _piece) :
            m_garbage.set(_x, -(_y + 1), _piece);
    }

#if __cplusplus >= 202002L
    constexpr
#endif
    piece get_number_at(i32 _x, i32 _y) const {
        return _y >= 0 ?
            m_field.get(_x, _y) :
            m_garbage.get(_x, -(_y + 1));
    }

#if __cplusplus >= 202002L
    constexpr
#endif
    piece get_number_at_index(u32 _idx, bool _is_field) const {
        return _is_field ?
            m_field.get(_idx % FIELD_WIDTH, _idx / FIELD_WIDTH) :
            m_garbage.get(_idx % FIELD_WIDTH, -(_idx / FIELD_WIDTH + 1));
    }

    const std::vector<piece>& field() const { return m_field.get_pieces(); }
    const std::vector<piece>& garbage() const { return m_garbage.get_pieces(); }
};

}