#pragma once

#include <string>

#include <stdexcept>

#include <details/intdef.hpp>

namespace fumen::details {

enum class piece_type : u8 {
    empty, I, L, O, Z, T, J, S, gray
};

enum class rotation_type : u8 {
    reverse, right, spawn, left
};

struct inner_operation {
    piece_type m_piece = piece_type::empty;
    rotation_type m_rotation = rotation_type::spawn;
    u32 m_x = 0, m_y = 0;
};

/* static */ class defs {
public:
    static constexpr bool is_mino(piece_type _piece) {
        u8 _value = static_cast<u8>(_piece);

        return 1u <= _value && _value <= 7u;
    }

    static constexpr char to_char(piece_type _piece) {
        switch (_piece) {
            case piece_type::empty: return '_';
            case piece_type::I:     return 'I';
            case piece_type::L:     return 'L';
            case piece_type::O:     return 'O';
            case piece_type::Z:     return 'Z';
            case piece_type::T:     return 'T';
            case piece_type::J:     return 'J';
            case piece_type::S:     return 'S';
            case piece_type::gray:  return 'X';
        }
        
        // For error handling
        return '\0';
    }

    static constexpr piece_type to_piece(char _ch) {
        switch (_ch) {
            case '_': return piece_type::empty;
            case 'I': return piece_type::I;
            case 'L': return piece_type::L;
            case 'O': return piece_type::O;
            case 'Z': return piece_type::Z;
            case 'T': return piece_type::T;
            case 'J': return piece_type::J;
            case 'S': return piece_type::S;
            case 'X': return piece_type::gray;
        }

        // For error handling
        return static_cast<piece_type>(9u);
    }
};

}