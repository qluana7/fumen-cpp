#pragma once

#include <vector>
#include <string>

#include <optional>

#include <details/intdef.hpp>
#include <details/strlib.hpp>

#include <details/defs.hpp>
#include <details/utils.hpp>
#include <details/buffer.hpp>
#include <details/action.hpp>
#include <details/comments.hpp>
#include <details/quiz.hpp>
#include <details/inner_field.hpp>
#include <details/field.hpp>

namespace fumen::details {

struct encode_page {
    std::optional<field> m_field;
    std::optional<field_operation> m_operation;
    std::optional<std::string> m_comment;

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

using encode_pages = std::vector<encode_page>;

/* static */ class encoder {
    static std::pair<bool, buffer> s_encode_field(
        const inner_field& _prev, const inner_field& _current
    ) {
        // In encode, only support { field = 23, garbage = 1 }
        u32 _field_top = 23,
            _max_height = _field_top + 1,
            _block_count = FIELD_WIDTH * _max_height;
        
        buffer _buf;

        auto _get_diff_fn = [&] (u32 _x, u32 _y) constexpr {
            u32 _py = _field_top - _y - 1;

            return
                static_cast<i8>(_current.get_number_at(_x, _py)) -
                static_cast<i8>(_prev.get_number_at(_x, _py)) + 8;
        };

        auto _record_block_counts_fn = [&] (i8 _diff, i64 _counter) {
            i64 _value = (i64)_diff * _block_count + _counter;
            _buf.push(_value, 2);
        };

        bool _changed = true;
        i8 _prev_diff = _get_diff_fn(0, 0);
        i64 _counter = -1;

        for (u32 _y = 0; _y < _max_height; _y++) {
            for (u32 _x = 0; _x < FIELD_WIDTH; _x++) {
                i8 _diff = _get_diff_fn(_x, _y);

                if (_diff != _prev_diff) {
                    _record_block_counts_fn(_prev_diff, _counter);
                    _counter = -1;
                    _prev_diff = _diff;
                }

                _counter++;
            }
        }

        _record_block_counts_fn(_prev_diff, _counter);

        if (_prev_diff == 8 && _counter == _block_count - 1)
            _changed = false;
        
        return { _changed, _buf };
    }

    static void s_update_field(
        buffer& _buf, i64& _last_ridx,
        const inner_field& _prev, const inner_field& _current
    ) {
        auto [_changed, __bufv] = s_encode_field(_prev, _current);

        if (_changed) {
            _buf += __bufv;
            _last_ridx = -1;
        } else if (_last_ridx < 0 || _buf[_last_ridx] == buffer::table_size - 1) {
            _buf += __bufv;
            _buf.push(0);
            _last_ridx = _buf.size() - 1;
        } else if (_buf[_last_ridx] < buffer::table_size - 1) {
            _buf[_last_ridx]++;
        }
    }

public:
    static std::string encode(const encode_pages& _pages) {
        i64 _last_ridx = -1;
        buffer _buf;
        inner_field _prev_field;

        action_codec _act_codec(FIELD_WIDTH, 23, GARBAGE_LINE);
        comment_codec _comment_codec;

        std::optional<std::string> _prev_comment = "";
        std::optional<quiz> _prev_quiz = std::nullopt;

        for (u32 _idx = 0; _idx < _pages.size(); _idx++) {
            auto& _current_page = _pages[_idx];
            std::optional<field> _field = _current_page.m_field;

            inner_field _current_field =
                _field ? inner_field(*_field) : _prev_field;
            
            s_update_field(_buf, _last_ridx, _prev_field, _current_field);

            std::optional<std::string> _current_comment = std::nullopt;

            if (_current_page.m_comment.has_value() &&
                (_idx != 0 || !_current_page.m_comment->empty()))
                _current_comment = *_current_page.m_comment;
            
            inner_operation _piece = _current_page.m_operation ?
                inner_operation {
                    defs::to_piece(_current_page.m_operation->m_piece),
                    defs::to_rotation(_current_page.m_operation->m_rotation),
                    _current_page.m_operation->m_x,
                    _current_page.m_operation->m_y
                } : inner_operation{ piece::empty, rotation::reverse, 0, 22 };
            
            std::optional<std::string> _next_comment = std::nullopt;

            if (_current_comment.has_value()) {
                if (strlib::startswith(*_current_comment, "#Q=")) {
                    if (!_prev_quiz.has_value() ||
                        _prev_quiz->format().to_string() != *_current_comment) {
                        _next_comment = _current_comment;
                        _prev_comment = _next_comment;
                        _prev_quiz = quiz(*_current_comment);
                    }
                } else {
                    if (_prev_quiz.has_value() &&
                        _prev_quiz->format().to_string() == *_current_comment) {
                            _prev_comment = _current_comment;
                            _prev_quiz = std::nullopt;
                    } else {
                        _next_comment = _prev_comment != _current_comment ? 
                            _current_comment : std::nullopt;
                        _prev_comment = _prev_comment != _current_comment ?
                            _next_comment : _prev_comment;
                        _prev_quiz = std::nullopt;
                    }
                }
            } else _prev_quiz = std::nullopt;

            if (_prev_quiz.has_value() &&
                _prev_quiz->can_operate() &&
                _current_page.m_flags.lock_bit
            ) {
                if (defs::is_mino(_piece.m_piece)) {
                    try {
                        quiz _next_quiz = _prev_quiz->next_if_end();
                        _prev_quiz = _next_quiz.operate(_next_quiz.get_operation(_piece.m_piece));
                    } catch (const std::exception& e) {
                        // No operation if an error occurs
                        _prev_quiz = _prev_quiz->format();
                    }
                } else _prev_quiz = _prev_quiz->format();
            }

            encode_page::flags _current_flags;
            _current_flags.lock_bit = true;
            _current_flags.colorize_bit = _idx == 0;
            _current_flags.all = _current_page.m_flags.all;

            action _act {
                _piece,
                static_cast<bool>(_current_flags.rise_bit),
                static_cast<bool>(_current_flags.mirror_bit),
                static_cast<bool>(_current_flags.colorize_bit),
                _next_comment.has_value(),
                static_cast<bool>(_current_flags.lock_bit)
            };

            i64 _act_num = _act_codec.encode(_act);
            _buf.push(_act_num, 3);

            if (_next_comment.has_value()) {
                std::string _estr = converter::escape(*_next_comment);
                u32 _comment_len = std::min<u32>(_estr.size(), 4095u);

                _buf.push(_comment_len, 2);

                for (u32 __i = 0; __i < _comment_len; __i += 4) {
                    i64 __v = 0;
                    for (u32 __cnt = 0; __cnt < 4; __cnt++) {
                        if (__i + __cnt >= _comment_len) break;

                        __v += _comment_codec.encode(_estr.at(__i + __cnt), __cnt);
                    }

                    _buf.push(__v, 5);
                }
            } else if (!_current_page.m_comment.has_value())
                _prev_comment = std::nullopt;
            
            if (_act.m_lock) {
                if (defs::is_mino(_act.m_operation.m_piece))
                    _current_field.fill(_act.m_operation);
                
                _current_field.clear_line();

                if (_act.m_rise)
                    _current_field.rise_garbage();
                
                if (_act.m_mirror)
                    _current_field.mirror();
            }

            _prev_field = _current_field;
        }

        std::string _data = _buf.to_string();
        if (_data.size() < 42) return _data;

        std::string _head = _data.substr(0, 42), _next;
        for (u32 __i = 0; ; __i++) {
            _next = _data.substr(42 + 47 * __i, 47);

            if (_next.empty()) break;

            _head += "?" + _next;

            if (_next.size() < 47) break;
        }

        return _head;
    }
};

}