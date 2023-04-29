#pragma once

#include <vector>
#include "Common.hpp"
#include "BitStream.hpp"

template<typename T>
class LosslessCompress {
public:
    virtual void Compress(const Ref<VectorX<T>>& vector, std::stringstream& out) const = 0;
    virtual void InverseCompress(std::stringstream& in, Ref<VectorX<T>> vector) const = 0;
};

template<typename T>
class DPCM : public LosslessCompress<T> {
public:
    void Compress(const Ref<VectorX<T>> &vector, std::stringstream &out) const override;
    void InverseCompress(std::stringstream &in, Ref<VectorX<T>> vector) const override;

protected:
    static void StoreLengthAndBits(int x, std::vector<unsigned char>& lengths, std::vector<T>& bits);
    static int ReadLengthAndBits(unsigned char length, T bits);
};

template<typename T>
class RLC : public LosslessCompress<T> {
public:
    void Compress(const Ref<VectorX<T>> &vector, std::stringstream &out) const override;
    void InverseCompress(std::stringstream &in, Ref<VectorX<T>> vector) const override;
};

template<typename T>
class LosslessCompressFactory {
public:
    static LosslessCompress<T>* GetLosslessCompress(const std::string& type) {
        if (type == "DPCM") {
            return new DPCM<T>;
        } else if (type == "RLC") {
            return new RLC<T>;
        } else {
            throw std::logic_error("Unimplemented lossless compress method");
        }
    }
};

template<typename T>
void DPCM<T>::Compress(const Ref<VectorX<T>> &vector, std::stringstream &out) const {
    const int size = vector.size();
    std::vector<unsigned char> lengths;
    std::vector<T> bits;
    StoreLengthAndBits(vector(0), lengths, bits);
    for (int i = 1; i < size; i++) {
        StoreLengthAndBits((int)vector(i) - (int)vector(i - 1), lengths, bits);
    }
    for (int i = 0; i < size; i++) {
        out.write(reinterpret_cast<const char*>(&lengths[i]), sizeof(unsigned char));
    }
    BitStream bit_stream;
    for (int i = 0; i < size; i++) {
        bit_stream.AddBits(lengths[i], bits[i]);
    }
    out << bit_stream;
}


template<typename T>
void DPCM<T>::InverseCompress(std::stringstream &in, Ref<VectorX<T>> vector) const {
    const int size = vector.size();
    std::vector<unsigned char> lengths;
    std::vector<T> bits;
    // Get lengths
    for (int i = 0; i < size; i++) {
        unsigned char length;
        in.read(reinterpret_cast<char*>(&length), sizeof(length));
        lengths.push_back(length);
    }

    // Get amplitudes
    BitStream bitstream;
    in >> bitstream;
    for (int i = 0, position = 0; i < size; i++) {
        bits.push_back(bitstream.GetBits<T>(position, lengths[i]));
        position += lengths[i];
    }

    vector(0) = ReadLengthAndBits(lengths[0], bits[0]);
    for (int i = 1; i < size; i++) {
        int diff = ReadLengthAndBits(lengths[i], bits[i]);
        vector(i) = vector(i - 1) + diff;
    }
}


template<typename T>
void DPCM<T>::StoreLengthAndBits(int x, std::vector<unsigned char>& lengths, std::vector<T>& bits) {
    bool reverse = false;
    if (x < 0) {
        x = -x;
        reverse = true;
    }
    int power = 1;
    unsigned char length = 0;
    for (; power <= x; power <<= 1, length++);
    lengths.push_back(length);
    bits.push_back(reverse ? ((power - 1) ^ x) : x);
}

template<typename T>
int DPCM<T>::ReadLengthAndBits(unsigned char length, T bits) {
    if (length == 0) {
        return 0;
    }
    if (bits & (1 << (length - 1))) {
        // positive
        return bits;
    } else {
        return -((~bits) & ((1 << length) - 1));
    }
}


template<typename T>
void RLC<T>::Compress(const Ref<VectorX<T>> &vector, std::stringstream &out) const {
    const int size = vector.size();
    unsigned char zero_count = 0;
    for (int index = 0; index < size; index++) {
        auto cur_value = vector(index);
        if (cur_value == 0 && zero_count < UCHAR_MAX) {
            zero_count++;
        } else {
            out.write(reinterpret_cast<const char*>(&zero_count), sizeof(zero_count));
            out.write(reinterpret_cast<const char*>(&cur_value), sizeof(cur_value));
            zero_count = 0;
        }
    }
    if (zero_count != 0) {
        zero_count--;
        T zero = 0;
        out.write(reinterpret_cast<const char*>(&zero_count), sizeof(zero_count));
        out.write(reinterpret_cast<const char*>(&zero), sizeof(zero));
    }
}

template<typename T>
void RLC<T>::InverseCompress(std::stringstream &in, Ref<VectorX<T>> vector) const {
    const int size = vector.size();
    int cur_position = 0;
    while(cur_position < size) {
        unsigned char zero_count;
        T non_zero_value;
        in.read(reinterpret_cast<char*>(&zero_count), sizeof(zero_count));
        in.read(reinterpret_cast<char*>(&non_zero_value), sizeof(non_zero_value));
        for (int i = 0; i < zero_count; i++) {
            vector(cur_position++) = 0;
        }
        vector(cur_position++) = non_zero_value;
    }
    assert(cur_position == size);
}
