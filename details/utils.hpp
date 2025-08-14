#pragma once

#include <format>

#include <vector>
#include <string>

#include <locale>

#include <details/intdef.hpp>

namespace fumen::details {

/* static */ class converter {
    static std::u16string utf8_to_utf16(const std::string& _str) {
        std::u16string _result;
        for (size_t _i = 0; _i < _str.size(); _i++) {
            unsigned char _current = _str[_i];
            u32 _temp = 0;

            u32 _counter = 0;
            // If '0xxxxxxx' (ASCII character)
            if (_current < 0x80)
            { _temp = _current; _counter = 0; }
            // If '110xxxxx 10xxxxxx' (2-byte character)
            else if (0xC2 <= _current && _current <= 0xDF)
            { _temp = _current & 0x1F; _counter = 1; }
            // If '1110xxxx 10xxxxxx 10xxxxxx' (3-byte character)
            else if (0xE0 <= _current && _current <= 0xEF)
            { _temp = _current & 0x0F; _counter = 2; }
            // If '11110xxx 10xxxxxx 10xxxxxx 10xxxxxx' (4-byte character)
            else if (0xF0 <= _current && _current <= 0xF4)
            { _temp = _current & 0x07; _counter = 3; }
            // Invalid UTF-8 byte
            else { _temp = 0xFFFD; _counter = 0; }

            for (u32 _j = 0; _j < _counter; _j++) {
                if (++_i >= _str.size()) { _temp = 0xFFFD; break; }
                
                _current = _str[_i];

                // detect overlong
                if (_j == 0) {
                    if (_counter == 2 && _current < 0xA0)
                    { _temp = 0xFFFD; break; }
                    else if (_counter == 3 && _current < 0x90)
                    { _temp = 0xFFFD; break; }
                }

                if (_current < 0x80 || 0xBF < _current)
                    { _temp = 0xFFFD; break; }

                _temp <<= 6;
                _temp |= _current & 0x3F;
            }

            if (0xD800 <= _temp && _temp <= 0xDFFF)
                _temp = 0xFFFD; // Invalid Unicode code point

            if (_temp < 0x10000)
                _result += (char16_t)_temp;
            else {
                _temp -= 0x10000;
                _result += (char16_t)(0xD800 | (_temp >> 10));
                _result += (char16_t)(0xDC00 | (_temp & 0x3FF));
            }
        }

        return _result;
    }

    static std::string utf16_to_utf8(const std::u16string& _str) {
        std::string _result;

        for (size_t _i = 0; _i < _str.size(); _i++) {
            char16_t _current = _str[_i];

            if (_current < 0x80) {
                _result += _current;
            } else if (_current < 0x800) {
                _result += (char)((_current >> 6) | 0xC0);
                _result += (char)((_current & 0x3F) | 0x80);
            } else if (0xD800 <= _current && _current <= 0xDFFF) {
                if (_i + 1 < _str.size()) {
                    char16_t _next = _str[_i + 1];
                    if (0xDC00 <= _next && _next <= 0xDFFF) {

                        uint32_t _cp = 0x10000 + (((_current - 0xD800) << 10) | (_next - 0xDC00));
                        _result += (char)((_cp >> 18) | 0xF0);
                        _result += (char)(((_cp >> 12) & 0x3F) | 0x80);
                        _result += (char)(((_cp >> 6) & 0x3F) | 0x80);
                        _result += (char)((_cp & 0x3F) | 0x80);
                        _i += 2;

                        continue;
                    }
                }

                _result += (char)0xEF;
                _result += (char)0xBF;
                _result += (char)0xBD;
                _i++;
                continue;
            }  else if (0xDC00 <= _current && _current <= 0xDFFF) {
                _result += (char)0xEF;
                _result += (char)0xBF;
                _result += (char)0xBD;
                _i++;
                continue;
            } else {
                _result += (char)((_current >> 12) | 0xE0);
                _result += (char)(((_current >> 6) & 0x3F) | 0x80);
                _result += (char)((_current & 0x3F) | 0x80);
            }
        }

        return _result;
    }

public:
    static std::string escape(const std::string& _str) {
        std::u16string _escstr = utf8_to_utf16(_str);
        std::string _result;

        for (char16_t  _c : _escstr) {
            if (
                std::isalnum(_c) ||
                ((std::u16string)u"@*_+-./").find(_c)
                    != std::string::npos
            ) _result += (char)_c;
            else {
                if (_c <= 0xff) {
#if __cplusplus >= 202002L || defined(__cpp_lib_format)
                    _result += "%" + std::format("{:02X}", (u16)_c);
#else
                    std::string _hexstr(2, '0');
                    std::sprintf(_hexstr.data(), "%02X", (u16)_c);
                    _result += "%" + _hexstr;
#endif
                }
                else {
#if __cplusplus >= 202002L || defined(__cpp_lib_format)
                    _result += "%u" + std::format("{:04X}", (u16)_c);
#else
                    std::string _hexstr(4, '0');
                    std::sprintf(_hexstr.data(), "%04X", (u16)_c);
                    _result += "%u" + _hexstr;
#endif
                }
            }
        }

        return _result;
    }

    static std::string unescape(const std::string& _str) {
        std::u16string _result;

        for (
            std::string::const_iterator _iter = _str.begin();
            _iter != _str.end();
            _iter++
        ) {
            char _c = *_iter;
            
            if (_c == '%') {
                _iter++;

                if (_iter == _str.end()) break;

                if (*_iter == 'u') {
                    std::string _hex; _iter++;

                    if (_iter == _str.end()) break;

                    for (u32 _i = 0; _i < 4; _i++) {
                        _hex += *_iter++;
                        if (_iter == _str.end()) break;
                    }

                    if (_hex.size() < 4) continue;

                    _result += (char16_t)std::stoul(_hex, nullptr, 16);
                } else {
                    std::string _hex;

                    for (u32 _i = 0; _i < 2; _i++) {
                        if (_iter == _str.end()) break;
                        _hex += *_iter++;
                    }

                    _iter--;

                    if (_hex.size() < 2) continue;

                    _result += (char16_t)std::stoul(_hex, nullptr, 16);
                }
            } else _result += (char16_t)_c;
        }

        return utf16_to_utf8(_result);
    }
};

}