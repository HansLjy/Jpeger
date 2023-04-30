#include "LosslessCompress.hpp"

void LosslessCompress::EntrophyEncoding(const std::vector<int>& data, std::stringstream& out) {
    int size = data.size();
    std::vector<unsigned char> lengths;
    BitStream bitstream;
    for (int i = 0; i < size; i++) {
        unsigned char length;
        int bits;
        GetLengthAndBits(data[i], length, bits);
        lengths.push_back(length);
        bitstream.AddBits<int>(length, bits);
    }
    out.write(reinterpret_cast<const char*>(&size), sizeof(size));
    for (int i = 0; i < size; i++) {
        out.write(reinterpret_cast<const char*>(&lengths[i]), sizeof(unsigned char));
    }
    out << bitstream;
}

void LosslessCompress::EntrophyDecoding(std::stringstream& in, std::vector<int>& data) {
    int size;
    in.read(reinterpret_cast<char*>(&size), sizeof(size));

    std::vector<unsigned char> lengths;
    for (int i = 0; i < size; i++) {
        unsigned char length;
        in.read(reinterpret_cast<char*>(&length), sizeof(length));
        lengths.push_back(length);
    }

    BitStream bitstream;
    in >> bitstream;
    int position = 0;
    for (int i = 0; i < size; i++) {
        int bits = bitstream.GetBits<int>(position, lengths[i]);
        data.push_back(ReadLengthAndBits(lengths[i], bits));
        position += lengths[i];
    }
}

void LosslessCompress::GetLengthAndBits(int x, unsigned char& length, int& bits) {
    bool reverse = false;
    if (x < 0) {
        x = -x;
        reverse = true;
    }
    int power = 1;
    length = 0;
    for (; power <= x; power <<= 1, length++);
    bits = reverse ? ((power - 1) ^ x) : x;
}

int LosslessCompress::ReadLengthAndBits(unsigned char length, int bits) {
    if (length == 0) {
        return 0;
    }
    if (bits & (1 << (length - 1))) {
        return bits;
    } else {
        return -((~bits) & ((1 << length) - 1));
    }
}

void DPCM::Compress(const Ref<VectorXi> &vector, std::stringstream &out) const {
    const int size = vector.size();
    std::vector<int> diffs;
    diffs.push_back(vector(0));
    for (int i = 1; i < size; i++) {
        diffs.push_back(vector(i) - vector(i - 1));
    }
    EntrophyEncoding(diffs, out);
}

void DPCM::InverseCompress(std::stringstream &in, Ref<VectorXi> vector) const {
    const int size = vector.size();
    std::vector<int> diffs;
    EntrophyDecoding(in, diffs);
    vector(0) = diffs[0];
    for (int i = 1; i < size; i++) {
        vector(i) = vector(i - 1) + diffs[i];
    }
}


void RLC::Compress(const Ref<VectorXi> &vector, std::stringstream &out) const {
    const int size = vector.size();
    int zero_count = 0;
    std::vector<int> zero_counts;
    std::vector<int> non_zero_values;
    for (int index = 0; index < size; index++) {
        auto cur_value = vector(index);
        if (cur_value == 0) {
            zero_count++;
        } else {
            zero_counts.push_back(zero_count);
            non_zero_values.push_back(cur_value);
            zero_count = 0;
        }
    }
    if (zero_count != 0) {
        zero_counts.push_back(zero_count - 1);
        non_zero_values.push_back(0);
    }
    EntrophyEncoding(zero_counts, out);
    EntrophyEncoding(non_zero_values, out);
}

void RLC::InverseCompress(std::stringstream &in, Ref<VectorXi> vector) const {
    std::vector<int> zero_counts;
    std::vector<int> non_zero_values;
    EntrophyDecoding(in, zero_counts);
    EntrophyDecoding(in, non_zero_values);
    int num_pairs = zero_counts.size();
    int cur_position = 0;
    for (int i = 0; i < num_pairs; i++) {
        int zero_count = zero_counts[i];
        for (int j = 0; j < zero_count; j++) {
            vector(cur_position++) = 0;
        }
        vector(cur_position++) = non_zero_values[i];
    }
}


LosslessCompress* LosslessCompressFactory::GetLosslessCompress(const std::string& type) {
    if (type == "DPCM") {
        return new DPCM;
    } else if (type == "RLC") {
        return new RLC;
    } else {
        throw std::logic_error("Unimplemented lossless compress method");
    }
}