#pragma once

#include <format>

#include <vector>
#include <string>

#include <algorithm>
#include <regex>

#include <details/intdef.hpp>
#include <details/strlib.hpp>

#include <details/defs.hpp>

namespace fumen::details {

enum class quiz_operation {
    direct, swap, stock
};

class quiz {
public:
    quiz() = default;
    quiz(const std::string& _data) : m_data(s_verify(_data)) {}

private:
    std::string m_data;

    static std::string s_verify(const std::string& _data) {
        std::string _str = strlib::strrmv(_data, [] (char c) {
            return !(c == ' ' || c == '\t' || c == '\n' || c == '\r');
        });

        if (_str.empty() || _data == "#Q=[]()" || !strlib::startswith(_data, "#Q="))
            return _data;
        
        std::regex _quiz_regex(
            R"(^#Q=\[[TIOSZJL]?]\([TIOSZJL]?\)[TIOSZJL]*;?.*$)",
            std::regex_constants::ECMAScript | std::regex_constants::icase
        );

        if (!std::regex_match(_str, _quiz_regex))
            throw std::invalid_argument("Invalid quiz format");
        
        return _str;
    }

#if __cplusplus >= 202002L
    constexpr
#endif
    char m_next() const {
        std::size_t _idx = m_data.find(')') + 1;
        if (_idx >= m_data.size() || m_data[_idx] == ';')
            return '\0';
        return m_data[_idx];
    }

    std::string m_least() const
    { return m_data.substr(m_data.find(')') + 1); }

#if __cplusplus >= 202002L
    constexpr
#endif
    char m_current() const {
        char _name = m_data[m_data.find('(') + 1];
        return _name == ')' ? '\0' : _name;
    }

#if __cplusplus >= 202002L
    constexpr
#endif
    char m_hold() const {
        char _name = m_data[m_data.find('[') + 1];
        return _name == ']' ? '\0' : _name;
    }

    std::string m_least_after_next2() const {
        std::size_t _idx = m_data.find(')');

        return m_data.substr(_idx + (m_data[_idx + 1] == ';' ? 1 : 2));
    }

    std::string m_least_in_active_bag() const {
        std::size_t _sepidx = m_data.find(';');
        std::string _str = 0 <= _sepidx ? m_data.substr(0, _sepidx) : m_data;
        std::size_t _idx = _str.find(')');

        return _str.substr(_idx + (_str[_idx + 1] == ';' ? 1 : 2));
    }

public:
    static quiz create(const std::string& _first, const std::string& _second) {
#if __cplusplus >= 202002L || defined(__cpp_lib_format)
        return quiz(std::format("#Q=[{}]({}){}", _first, _second[0], _second.substr(1)));
#else
        return quiz(std::string("#Q=[") + _first + "](" + _second[0] + ")" + _second.substr(1));
#endif
    }

    static quiz create(const std::string& _first) {
#if __cplusplus >= 202002L || defined(__cpp_lib_format)
        return quiz(std::format("#Q=[]({}){}", _first[0], _first.substr(1)));
#else
        return quiz(std::string("#Q=[](") + _first[0] + ")" + _first.substr(1));
#endif
    }

    static bool is_quiz_comment(const std::string& _str)
    { return strlib::startswith(_str, "#Q="); }

    quiz_operation get_operation(piece _piece) const {
        char
            _uname = defs::to_char(_piece),
            _cname = m_current();
        
        if (_uname == _cname) return quiz_operation::direct;

        char _hold = m_hold();
        if (_hold == _uname) return quiz_operation::swap;

        if (_hold == '\0') {
            if (_uname == m_next()) return quiz_operation::stock;
        } else {
            if (_cname == '\0' && _uname == m_next()) return quiz_operation::direct;
        }

        throw std::invalid_argument("Invalid hold piece");
    }

    quiz direct() const {
        if (m_current() == '\0') {
            std::string _least = m_least_after_next2();

#if __cplusplus >= 202002L || defined(__cpp_lib_format)
            return quiz(std::format("#Q=[{}]({}){}", m_hold(), _least[0], _least.substr(1)));
#else
            return quiz(std::string("#Q=[") + m_hold() + "](" + _least[0] + ")" + _least.substr(1));
#endif
        }

#if __cplusplus >= 202002L || defined(__cpp_lib_format)
        return quiz(std::format("#Q=[{}]({}){}", m_hold(), m_next(), m_least_after_next2()));
#else
        return quiz(std::string("#Q=[") + m_hold() + "](" + m_next() + ")" + m_least_after_next2());
#endif
    }

    quiz swap() const {
        if (m_hold() == '\0')
            throw std::runtime_error("Cannot swap with no hold piece");
        
#if __cplusplus >= 202002L || defined(__cpp_lib_format)
        return quiz(std::format("#Q=[{}]({}){}", m_current(), m_next(), m_least_after_next2()));
#else
        return quiz(std::string("#Q=[") + m_current() + "](" + m_next() + ")" + m_least_after_next2());
#endif
    }

    quiz stock() const {
        if (m_hold() != '\0' || m_next() == '\0')
            throw std::runtime_error("Cannot stock");
        
        std::string _least = m_least_after_next2();
        std::string _head = _least.empty() ? std::string() : _least.substr(0, 1);

        if (_least.size() > 1) {
#if __cplusplus >= 202002L || defined(__cpp_lib_format)
            return quiz(std::format("#Q=[{}]({}){}", m_hold(), _head, _least.substr(1)));
#else
            return quiz(std::string("#Q=[") + m_hold() + "](" + _head + ")" + _least.substr(1));
#endif
        } else {
#if __cplusplus >= 202002L || defined(__cpp_lib_format)
            return quiz(std::format("#Q=[{}]({})", m_hold(), _head));
#else
            return quiz(std::string("#Q=[") + m_hold() + "](" + _head + ")");
#endif
        }
    }

    quiz operate(quiz_operation _op) const {
        switch (_op) {
            case quiz_operation::direct: return direct();
            case quiz_operation::swap:   return swap();
            case quiz_operation::stock:  return stock();
            default: break;
        }

        throw std::invalid_argument("Invalid operation type");
    }

    quiz format() const {
        quiz _quiz = next_if_end();

        if (_quiz.m_data == "#Q=[]()")
            return quiz("");
        
        char
            _current = m_current(),
            _hold = m_hold();
        
        if (_current == '\0' && _hold == '\0') {
#if __cplusplus >= 202002L || defined(__cpp_lib_format)
            return quiz(std::format("#Q=[]({}){}", _hold, _quiz.m_least()));
#else
            return quiz(std::string("#Q=[](") + _hold + ")" + _quiz.m_least());
#endif
        }
        
        if (_current == '\0') {
            std::string _least = _quiz.m_least();
            if (_least.empty()) return quiz("");

            char _head = _least[0];
            if (_head == ';') return quiz(_least.substr(1));

#if __cplusplus >= 202002L || defined(__cpp_lib_format)
            return quiz(std::format("#Q=[]({}){}", _head, _least.substr(1)));
#else
            return quiz(std::string("#Q=[](") + _head + ")" + _least.substr(1));
#endif
        }

        return _quiz;
    }

    piece get_hold() const {
        if (!can_operate()) return piece::empty;

        char _hold = m_hold();

        if (_hold == '\0' || _hold == ';')
            return piece::empty;
        
        return defs::to_piece(_hold);
    }

    std::vector<piece> get_nexts(i64 _max = 0) const {
        if (!can_operate()) return std::vector<piece>(_max, piece::empty);

        std::string _name =
            (std::string(m_current(), 1) + m_next() + m_least_in_active_bag())
                .substr(0, _max);
        
        if (_max != 0 && (i64)_name.size() < _max)
            _name += std::string(_max - _name.size(), ' ');
        
        std::vector<piece> _pieces(_name.size());
        std::transform(_name.begin(), _name.end(), _pieces.begin(), [] (char __c) {
            if (__c == '\0' || __c == ' ' || __c == ';') return piece::empty;
            else return defs::to_piece(__c);
        });

        return _pieces;
    }

    std::string to_string() const { return m_data; }

    bool can_operate() const {
        std::string _quiz = m_data;

        if (strlib::startswith(_quiz, "#Q=[]();"))
            _quiz = m_data.substr(8);
        
        return strlib::startswith(_quiz, "#Q=") && _quiz != "#Q=[]()";
    }

    quiz next_if_end() const {
        if (strlib::startswith(m_data, "#Q=[]();"))
            return quiz(m_data.substr(8));
        
        return *this;
    }
};

}