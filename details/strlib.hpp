#pragma once

#include <string>
#include <numeric>
#include <vector>
#include <sstream>
#include <functional>
#include <type_traits>
#include <algorithm>

#include <cstddef>

class strlib {
private:
    template <typename Iterator>
    using t_string_arguments = typename std::enable_if_t<std::is_same<typename std::iterator_traits<Iterator>::value_type, std::string>::value>;

public:
    template <typename Predicate>
    inline static std::size_t find_not_if(const std::string& _src, Predicate _pred) {
        std::string::const_iterator _begin = _src.cbegin(), _end = _src.cend();
        std::size_t _pos = 0;

        for (; _begin != _end && _pred(*_begin); _begin++, _pos++);

        return _pos;
    }

    template <typename Predicate>
    inline static std::size_t rfind_not_if(const std::string& _src, Predicate _pred) {
        std::string::const_reverse_iterator _begin = _src.crbegin(), _end = _src.crend();
        std::size_t _pos = _src.size();

        for (; _begin != _end && _pred(*_begin); _begin++, _pos--);

        return _pos;
    }

    template <typename Iterator, typename = t_string_arguments<Iterator>>
    inline static std::string join(Iterator _begin, Iterator _end, const std::string& _delim = "") {
        if (_begin == _end) return "";

        std::string _output = *_begin;

        for (_begin++; _begin != _end; _begin++)
            _output += _delim + *_begin;
        
        return _output;
    }

    template <typename Iterator, typename UnaryOp>
    inline static std::string join(Iterator _begin, Iterator _end, UnaryOp _conv_fn, const std::string& _delim = "") {
        if (_begin == _end) return "";

        std::string _output = _conv_fn(*_begin);

        for (_begin++; _begin != _end; _begin++)
            _output += _delim + _conv_fn(*_begin);

        return _output;
    }

    template <typename Iterator, typename UnaryOp>
    inline static std::string join_as_iter(Iterator _begin, Iterator _end, UnaryOp _conv_fn, const std::string& _delim = "") {
        if (_begin == _end) return "";

        std::string _output = _conv_fn(_begin);

        for (_begin++; _begin != _end; _begin++)
            _output += _delim + _conv_fn(_begin);

        return _output;
    }

    template <typename Iterator, typename = t_string_arguments<Iterator>>
    [[ deprecated ]]
    inline static size_t split(const std::string& _src, Iterator _dest, char _delim) {
        size_t _counts = 0;

        std::stringstream _buffer(_src);
        
        for (std::string _tok; std::getline(_buffer, _tok, _delim); _counts++, _dest++)
            *_dest = _tok;
        
        return _counts;
    }

    [[ deprecated ]]
    inline static void split(const std::string& _src, std::vector<std::string>& _dest, char _delim) {
        std::stringstream _buffer(_src);
        
        for (std::string _tok; std::getline(_buffer, _tok, _delim); _dest.push_back(_tok));
    }

    template <typename OutT, typename UnaryOp>
    [[ deprecated ]]
    inline static std::vector<OutT> split_map(const std::string& _src, UnaryOp _conv_fn, char _delim) {
        std::stringstream _buffer(_src);
        std::vector<OutT> _result;
        std::string _tok;

        while (std::getline(_buffer, _tok, _delim))
            _result.push_back(_conv_fn(_tok));

        return _result;
    }

    template <typename Container>
    inline static Container split(const std::string& _src, char _delim) {
        std::stringstream _buffer(_src);
        Container _cont; auto _iter = std::back_inserter(_cont);
        std::string _tok;

        while (getline(_buffer, _tok, _delim))
            _iter = _tok;

        return _cont;
    }

    template <typename Container, typename UnaryOp>
    inline static Container split_map(const std::string& _src, UnaryOp _conv_fn, char _delim) {
        std::stringstream _buffer(_src);
        Container _cont; auto _iter = std::back_inserter(_cont);
        std::string _tok;

        while (std::getline(_buffer, _tok, _delim))
            _iter = _conv_fn(_tok);

        return _cont;
    }

    template <typename UnaryFunc>
    inline static void split_foreach(const std::string& _src, UnaryFunc _unary_fn, char _delim) {
        std::stringstream _buffer(_src);
        std::string _tok;

        while (std::getline(_buffer, _tok, _delim))
            _unary_fn(_tok);
    }

    // Remove character if predict return false.
    template <typename Predict>
    inline static std::string strrmv(const std::string& _src, Predict _pred) {
        std::string _result; _result.resize(_src.size());

        std::size_t _i = 0;
        for (auto _ch : _src) {
            if (!_pred(_ch)) continue;

            _result[_i] = _ch; _i++;
        }

        _result.resize(_i);

        return _result;
    }

    inline static bool empty_or_space(const std::string& _src) {
        for (auto _ch : _src)
            if (!std::isspace(_ch)) return false;

        return true;
    }

    inline static std::string ltrim(const std::string& _src)
    { return _src.substr(find_not_if(_src, (int (*)(int))std::isspace)); }

    inline static std::string rtrime(const std::string& _src)
    { return _src.substr(0, rfind_not_if(_src, (int (*)(int))std::isspace)); }

    inline static std::string trim(const std::string& _src)
    { return ltrim(rtrime(_src)); }

    inline static std::string pad_left(const std::string& _src, int _cnt, char _ch)
    { return std::string(std::max<int>(0, _cnt - _src.size()), _ch) + _src; }

    inline static std::string pad_right(const std::string& _src, int _cnt, char _ch)
    { return _src + std::string(std::max<int>(0, _cnt - _src.size()), _ch); }

    inline static bool startswith(const std::string& _src, const std::string& _pat) {
#if __cplusplus >= 202002L || defined(__cpp_lib_starts_ends_with)
        return _src.starts_with(_pat);
#else
        if (_pat.size() > _src.size()) return false;

        for (size_t _i = 0; _i < _pat.size(); _i++)
            if (_src[_i] != _pat[_i]) return false;

        return true;
#endif
    }

    inline static bool endswith(const std::string& _src, const std::string& _pat) {
#if __cplusplus >= 202002L || defined(__cpp_lib_starts_ends_with)
        return _src.ends_with(_pat);
#else
        if (_pat.size() > _src.size()) return false;

        for (size_t _i = 0; _i < _pat.size(); _i++)
            if (_src[_src.size() - _i - 1] != _pat[_pat.size() - _i - 1])
                return false;
        
        return true;
#endif
    }

    inline static std::string toupper(const std::string& _src) {
        std::string _result(_src);
        
        for (auto& _ch : _result)
            _ch = std::toupper(_ch);
        
        return _result;
    }

    inline static std::string tolower(const std::string& _src) {
        std::string _result(_src);
        
        for (auto& _ch : _result)
            _ch = std::tolower(_ch);
            
        return _result;
    }

    inline static bool try_parse(int& _dest, const std::string& _src, int _base = 10) {
        // TODO : Improve performance to replace std::stoi to std::strtol
        //        -> Remove try-catch statement.
        try {
            size_t _idx;
            _dest = std::stoi(_src, &_idx, _base);
            return _idx == _src.size();
        } catch (...) { return false; }
    }

    inline static bool is_digit(const std::string& _src) {
        for (auto _ch : _src)
            if (!std::isdigit(_ch)) return false;

        return true;
    }

    inline static bool is_alpha(const std::string& _src) {
        for (auto _ch : _src)
            if (!std::isalpha(_ch)) return false;

        return true;
    }

    inline static bool is_alnum(const std::string& _src) {
        for (auto _ch : _src)
            if (!std::isalnum(_ch)) return false;

        return true;
    }
};
