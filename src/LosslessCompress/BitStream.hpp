#pragma once

#include <vector>
#include <iostream>
#include <sstream>
#include <cassert>

class BitStream;
std::stringstream& operator<<(std::stringstream& out, const BitStream& bitstream);

class BitStream {
public:
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
    T GetBits(int position, unsigned char length) {
        int cur_block = position / 8;
        unsigned char cur_useful_bits = (cur_block >= _data.size() ? 8 - _current_free_bits : 8) - (position & 7);

        T result = 0;
        while (cur_block < _data.size() && cur_useful_bits < length) {
            result = (result << cur_useful_bits) + (_data[cur_block] & ((1 << cur_useful_bits) - 1));
            length -= cur_useful_bits;
            cur_block++;
            cur_useful_bits = cur_block < _data.size() ? 8 : 8 - _current_free_bits;
        }
        if (cur_block < _data.size()) {
            result = (result << length) + ((_data[cur_block] >> (cur_useful_bits - length)) & ((1 << length) - 1));
        } else {
            assert(length <= cur_useful_bits);
            result = (result << length) + ((_current_data >> (cur_useful_bits - length)) & ((1 << length) - 1));
        }
        return result;
    }

    friend std::stringstream& operator<<(std::stringstream& out, const BitStream& bitstream);
    friend std::stringstream& operator>>(std::stringstream& in, BitStream& bitstream);

protected:
    std::vector<unsigned char> _data;
    unsigned char _current_free_bits = 8;
    unsigned char _current_data;
};

std::stringstream& operator<<(std::stringstream& out, const BitStream& bitstream);
std::stringstream& operator>>(std::stringstream& in, BitStream& bitstream);
