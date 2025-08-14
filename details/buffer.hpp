#pragma once

#include <deque>
#include <string>
#include <string_view>

#include <stdexcept>

#include <details/math.hpp>
#include <details/intdef.hpp>

namespace fumen::details {

class buffer {
public:
    buffer() = default;
    buffer(const std::string& _data) {
        for (char _c : _data)
            m_data.push_back(s_single_decode(_c));
    }

    typedef u8 value_type;
    typedef std::deque<value_type>::iterator iterator;
    typedef std::deque<value_type>::const_iterator const_iterator;
    typedef std::deque<value_type>::reverse_iterator reverse_iterator;
    typedef std::deque<value_type>::const_reverse_iterator const_reverse_iterator;
    typedef std::deque<value_type>::size_type size_type;
    typedef std::deque<value_type>::reference reference;
    typedef std::deque<value_type>::const_reference const_reference;

private:
    std::deque<value_type> m_data;

    static constexpr std::string_view s_table =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    static constexpr char _S_single_encode(value_type _c)
    { return s_table[_c]; }

    static constexpr value_type s_single_decode(char _c) {
        if (_c >= 'A' && _c <= 'Z') return _c - 'A';
        if (_c >= 'a' && _c <= 'z') return _c - 'a' + 26;
        if (_c >= '0' && _c <= '9') return _c - '0' + 52;
        if (_c == '+') return 62;
        if (_c == '/') return 63;
        return 0; // Invalid character
    }

public:
    static constexpr size_type table_size = s_table.size();

    /* Iterators */
    iterator               begin()         { return m_data.begin();   }
    iterator               end()           { return m_data.end();     }
    reverse_iterator       rbegin()        { return m_data.rbegin();  }
    reverse_iterator       rend()          { return m_data.rend();    }
    const_iterator         begin()   const { return m_data.begin();   }
    const_iterator         end()     const { return m_data.end();     }
    const_reverse_iterator rbegin()  const { return m_data.rbegin();  }
    const_reverse_iterator rend()    const { return m_data.rend();    }
    const_iterator         cbegin()  const { return m_data.cbegin();  }
    const_iterator         cend()    const { return m_data.cend();    }
    const_reverse_iterator crbegin() const { return m_data.crbegin(); }
    const_reverse_iterator crend()   const { return m_data.crend();   }

    /* Converter */
    std::string to_string() const {
        std::string _result;
        _result.reserve(m_data.size());

        for (const value_type& _c : m_data)
            _result.push_back(_S_single_encode(_c));

        return _result;
    }

    /* Modifiers */
    i64 poll(u32 _max) {
        i64 _value = 0;

        for (u32 _i = 0; _i < _max; _i++) {
            if (m_data.empty())
                throw std::invalid_argument("Invalid fumen data");

            value_type _d = m_data.front(); m_data.pop_front();

            _value += _d * math::powi<i64>(s_table.size(), _i);
        }

        return _value;
    }

    void push(i64 _value, u32 _cnt = 1) {
        for (u32 _i = 0; _i < _cnt; _i++) {
            m_data.push_back(_value % s_table.size());
            _value /= s_table.size();
        }
    }

    void merge(const buffer& _other) {
        for (const value_type& _c : _other.m_data)
            m_data.push_back(_c);
    }

    /* Capacity */
    bool empty() const { return m_data.empty(); }
    size_type size() const { return m_data.size(); }

    /* Accessor */
    reference at(size_type _idx) {
        if (_idx >= m_data.size())
            throw std::out_of_range("Index out of range in buffer");

        auto _it = m_data.begin();
        std::advance(_it, _idx);
        return *_it;
    }

    const_reference at(size_type _idx) const {
        if (_idx >= m_data.size())
            throw std::out_of_range("Index out of range in buffer");

        auto _it = m_data.begin();
        std::advance(_it, _idx);
        return *_it;
    }

    /* Operators */
    reference operator[] (size_type _idx) { return at(_idx); }

    const_reference operator[] (size_type _idx) const { return at(_idx); }

    buffer operator+(const buffer& _other) const {
        buffer _result = *this;
        _result.merge(_other);
        return _result;
    }
    
    buffer& operator+=(const buffer& _other) {
        merge(_other);
        return *this;
    }
};

}