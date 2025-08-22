#pragma once

#include <vector>
#include <string>

#include <regex>
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

struct page {
    u32 m_idx;
    inner_field m_inner_field;
    std::optional<field_operation> m_operation;
    std::optional<std::string> m_comment;
    struct {
        std::optional<u32> m_field, m_comment;
    } m_refs;
    union flags {
        struct {
            u8 lock_bit : 1;
            u8 mirror_bit : 1;
            u8 colorize_bit : 1;
            u8 rise_bit : 1;
            u8 quiz_bit : 1;
            u8 reserved : 3;
        };
        u8 all = 0;
    } m_flags;
};

using pages = std::vector<page>;

/* static */ class decoder {
private:
    struct store_data {
        i32 m_counter = -1;
        struct {
            i32 m_field = 0, m_comment = 0;
        } m_refs;
        std::optional<quiz> m_quiz = std::nullopt;
        std::string m_last_comment = "";
    };

    static std::pair<u32, std::string> s_extract(const std::string& _data) {
        auto rmf = [] (char c) {
            return !(c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '?');
        };

        std::string _dt = _data;

        size_t _idx = _dt.find('&');
        if (_idx != std::string::npos)
            _dt = _dt.substr(0, _idx);

        std::regex _ver_regx("([vmd])(110|115)@", std::regex::ECMAScript);
        std::smatch _matches;
        if (!std::regex_search(_dt, _matches, _ver_regx))
            throw std::logic_error("Unsupported Fumen version.");

        return {
            std::stoi(_matches.str().substr(1, 3)),
            strlib::strrmv(_matches.suffix().str(), rmf)
        };
    }

    static std::pair<bool, inner_field> s_update_field(
        buffer& _buf,
        const u32 _htop, const u32 _block_count,
        const inner_field& _prev
    ) {
        bool _is_changed = true; inner_field _field = _prev;

        u32 _idx = 0;

        while (_idx < _block_count) {
            i64 _block_diff = _buf.poll(2),
                _diff = _block_diff / _block_count,
                _counts = _block_diff % _block_count;
            
            if (_diff == 8 && _counts == _block_count - 1)
                _is_changed = false;
            
            for (u32 _i = 0; _i < _counts + 1; _i++) {
                i32 _x = _idx % FIELD_WIDTH,
                    _y = _htop - _idx / FIELD_WIDTH - 1;
                _field.add_number(_x, _y, _diff - 8);
                _idx++;
            }
        }

        return { _is_changed, _field };
    }

    static pages s_decode(const std::string& _data, u32 _htop) {
        u32 _max_height = _htop + GARBAGE_LINE,
            _block_count = FIELD_WIDTH * _max_height;

        buffer _buf(_data);

        u32 _pidx = 0;
        inner_field _prev_field;

        store_data _st_data;

        pages _pages;
        action_codec _act_codec(FIELD_WIDTH, _htop, GARBAGE_LINE);
        comment_codec _comment_codec;

        while (!_buf.empty()) {
            std::pair<bool, inner_field> _current;

            if (0 < _st_data.m_counter) {
                _current = { false, _prev_field };

                _st_data.m_counter--;
            } else {
                _current = s_update_field(_buf, _htop, _block_count, _prev_field);

                if (!_current.first)
                    _st_data.m_counter = _buf.poll(1);
            }

            action _act = _act_codec.decode(_buf.poll(3));

            std::pair<std::optional<std::string>, std::optional<i32>> _comment;
            if (_act.m_comment) {
                std::string _comment_string;
                i64 _comment_len = _buf.poll(2);

                for (i64 _i = 0; _i < (_comment_len + 3) / 4; _i++)
                    _comment_string += _comment_codec.decode(_buf.poll(5));
                
                _comment_string.resize(_comment_len);
                _comment_string = converter::unescape(_comment_string);
                _st_data.m_last_comment = _comment_string;
                _comment.first = _comment_string;
                _st_data.m_refs.m_comment = _pidx;

                if (quiz::is_quiz_comment(_comment_string)) {
                    try {
                        _st_data.m_quiz = quiz(_comment_string);
                    } catch (const std::invalid_argument&) {
                        _st_data.m_quiz = std::nullopt;
                    }
                } else
                    _st_data.m_quiz = std::nullopt;
            } else if (_pidx == 0)
                _comment.first = "";
            else {
                if (_st_data.m_quiz.has_value())
                    _comment.first = _st_data.m_quiz->format().to_string();
                else
                    _comment.first = std::nullopt;
                
                _comment.second = _st_data.m_refs.m_comment;
            }

            bool _is_quiz = _st_data.m_quiz.has_value();
            if (_is_quiz && _st_data.m_quiz->can_operate() && _act.m_lock) {
                if (defs::is_mino(_act.m_operation.m_piece)) {
                    try {
                        quiz _next = _st_data.m_quiz->next_if_end();
                        _st_data.m_quiz = _next.operate(
                            _next.get_operation(_act.m_operation.m_piece)
                        );
                    } catch (const std::invalid_argument&) {
                        _st_data.m_quiz = _st_data.m_quiz->format();
                    }
                }
            }
            
            page _page;
            _page.m_idx = _pidx;
            _page.m_inner_field = _current.second;
            if (_act.m_operation.m_piece != piece::empty) {
                _page.m_operation = static_cast<field_operation>(
                    mino(
                        _act.m_operation.m_piece,
                        _act.m_operation.m_rotation,
                        _act.m_operation.m_x,
                        _act.m_operation.m_y
                    )
                );
            } else
                _page.m_operation = std::nullopt;
            _page.m_comment = _comment.first.has_value() ?
                *_comment.first : _st_data.m_last_comment;
            _page.m_flags = {
                .lock_bit = _act.m_lock,
                .mirror_bit = _act.m_mirror,
                .colorize_bit = _act.m_colorize,
                .rise_bit = _act.m_rise,
                .quiz_bit = _is_quiz
            };
            if (_comment.second.has_value())
                _page.m_refs.m_comment = *_comment.second;
            else
                _page.m_refs.m_comment = std::nullopt;
            if (_current.first || _pidx == 0) {
                _st_data.m_refs.m_field = _pidx;
                _page.m_refs.m_field = std::nullopt;
            } else
                _page.m_refs.m_field = _st_data.m_refs.m_field;
            
            _pages.push_back(_page);

            _pidx++;

            if (_act.m_lock) {
                if (defs::is_mino(_act.m_operation.m_piece))
                    _current.second.fill(_act.m_operation);
                
                _current.second.clear_line();

                if (_act.m_rise)
                    _current.second.rise_garbage();
                
                if (_act.m_mirror)
                    _current.second.mirror();
            }

            _prev_field = _current.second;
        }

        return _pages;
    }

public:
    static pages decode(const std::string& _data) {
        auto [__v, _dt] = s_extract(_data);
        
        return s_decode(_dt, __v == 115 ? 23 : 21);
    }
};

}