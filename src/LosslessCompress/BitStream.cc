#include "BitStream.hpp"

std::stringstream& operator<<(std::stringstream& out, const BitStream& bitstream) {
    const int total_length = bitstream._data.size();
    out.write(reinterpret_cast<const char*>(&total_length), sizeof(total_length));
    for (const unsigned char data : bitstream._data) {
        out.write(reinterpret_cast<const char*>(&data), sizeof (data));
    }
    out.write(reinterpret_cast<const char*>(&bitstream._current_data), sizeof(bitstream._current_data));
    out.write(reinterpret_cast<const char*>(&bitstream._current_free_bits), sizeof(bitstream._current_free_bits));
    return out;
}

std::stringstream& operator>>(std::stringstream& in, BitStream& bitstream) {
    int total_length;
    in.read(reinterpret_cast<char*>(&total_length), sizeof(total_length));
    for (int i = 0; i < total_length; i++) {
        unsigned char data;
        in.read(reinterpret_cast<char*>(&data), sizeof(data));
        bitstream._data.push_back(data);
    }
    in.read(reinterpret_cast<char*>(&bitstream._current_data), sizeof(bitstream._current_data));
    in.read(reinterpret_cast<char*>(&bitstream._current_free_bits), sizeof(bitstream._current_free_bits));
    return in;
}
