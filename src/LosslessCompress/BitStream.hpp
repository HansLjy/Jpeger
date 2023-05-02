#pragma once

#include <vector>
#include <iostream>
#include <sstream>
#include <cassert>

class BitStream;
std::stringstream& operator<<(std::stringstream& out, const BitStream& bitstream);

class BitStream {
public:
    int GetSize() const {
        return _data.size() * sizeof(unsigned char) + 5;
    }

    template<typename T>
    void AddBits(unsigned char length, T val) {
        while (_current_free_bits < length) {
            length -= _current_free_bits;
            _current_data = (_current_data << _current_free_bits) + (val >> length);
            val -= ((val >> length) << length);

            _data.push_back(_current_data);
            _current_data = 0;
            _current_free_bits = 8;
        }
        _current_data = (_current_data << length) + val;
        _current_free_bits -= length;
    }

    template<typename T>
    T GetBits(unsigned char length) {
        unsigned char cur_useful_bits = (_cur_block >= _data.size() ? 8 - _current_free_bits : 8) - _cur_position;

        T result = 0;
        while (_cur_block < _data.size() && cur_useful_bits < length) {
            result = (result << cur_useful_bits) + (_data[_cur_block] & ((1 << cur_useful_bits) - 1));
            length -= cur_useful_bits;
            _cur_block++;
            cur_useful_bits = _cur_block < _data.size() ? 8 : 8 - _current_free_bits;
        }
        if (_cur_block < _data.size()) {
            result = (result << length) + ((_data[_cur_block] >> (cur_useful_bits - length)) & ((1 << length) - 1));
            _cur_position = 8 - cur_useful_bits + length;
        } else {
            assert(length <= cur_useful_bits);
            result = (result << length) + ((_current_data >> (cur_useful_bits - length)) & ((1 << length) - 1));
            _cur_position = 8 - _current_free_bits - cur_useful_bits + length;
        }
        return result;
    }

    bool GetBit() {
        bool result = _cur_block == _data.size()
            ? ((_current_data >> (7 - _current_free_bits - _cur_position)) & 1)
            : ((_data[_cur_block] >> (7 - _cur_position)) & 1);
        if (_cur_position < 7) {
            _cur_position++;
        } else {
            _cur_position = 0;
            _cur_block++;
        }
        return result;
    }

    friend std::stringstream& operator<<(std::stringstream& out, const BitStream& bitstream);
    friend std::stringstream& operator>>(std::stringstream& in, BitStream& bitstream);

    std::vector<unsigned char> _data;
    unsigned char _current_free_bits = 8;
    unsigned char _current_data;
    int _cur_block = 0;
    int _cur_position = 0;
};

std::stringstream& operator<<(std::stringstream& out, const BitStream& bitstream);
std::stringstream& operator>>(std::stringstream& in, BitStream& bitstream);
