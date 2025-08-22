#pragma once

#include <string>

#include <stdexcept>

#include <details/intdef.hpp>

namespace fumen::details {

enum class piece : u8 {
    empty, I, L, O, Z, T, J, S, gray
};

enum class rotation : u8 {
    reverse, right, spawn, left
};

struct inner_operation {
    piece m_piece = piece::empty;
    rotation m_rotation = rotation::spawn;
    u32 m_x = 0, m_y = 0;
};

/* static */ class defs {
public:
    static constexpr bool is_mino(piece _piece) {
        u8 _value = static_cast<u8>(_piece);

        return 1u <= _value && _value <= 7u;
    }

    static constexpr char to_char(piece _piece) {
        switch (_piece) {
            case piece::empty: return '_';
            case piece::I:     return 'I';
            case piece::L:     return 'L';
            case piece::O:     return 'O';
            case piece::Z:     return 'Z';
            case piece::T:     return 'T';
            case piece::J:     return 'J';
            case piece::S:     return 'S';
            case piece::gray:  return 'X';
        }
        
        // For error handling
        return '\0';
    }

    static constexpr piece to_piece(char _ch) {
        switch (_ch) {
            case '_': return piece::empty;
            case 'I': return piece::I;
            case 'L': return piece::L;
            case 'O': return piece::O;
            case 'Z': return piece::Z;
            case 'T': return piece::T;
            case 'J': return piece::J;
            case 'S': return piece::S;
            case 'X': return piece::gray;
        }

        // For error handling
        return static_cast<piece>(9u);
    }
};

}