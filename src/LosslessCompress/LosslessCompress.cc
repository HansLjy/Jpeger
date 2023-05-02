#include "LosslessCompress.hpp"
#include <queue>

namespace HuffmanCoding {
    struct TreeNode {
        TreeNode(
            const std::shared_ptr<TreeNode>& left,
            const std::shared_ptr<TreeNode>& right,
            int val
        ) : _left(left), _right(right), _val(val) {}

        std::shared_ptr<TreeNode> _left = nullptr;
        std::shared_ptr<TreeNode> _right = nullptr;
        int _val;   // -1 for non-leaf
    };

    struct Record {
        int _frequency;
        std::shared_ptr<TreeNode> _node;
        bool operator<(const Record& rhs) const {
            return _frequency > rhs._frequency; // inverse order for pq
        }
    };

    Record Merge(const Record& left, const Record& right) {
        return {
            left._frequency + right._frequency, std::make_shared<TreeNode>(
                left._node, right._node, -1
            )
        };
    }

    void AddCode(TreeNode& node, int code, unsigned char code_length, unsigned char plain_text) {
        if (code_length == 0) {
            node._val = plain_text;
            return;
        }
        switch ((code >> (code_length - 1)) & 1) {
            case 0: {
                if (!node._left) {
                    node._left = std::make_shared<TreeNode>(nullptr, nullptr, -1);
                }
                AddCode(*node._left, code, code_length - 1, plain_text);
                break;
            }
            case 1: {
                if (!node._right) {
                    node._right = std::make_shared<TreeNode>(nullptr, nullptr, -1);
                }
                AddCode(*node._right, code - (1 << (code_length - 1)), code_length - 1, plain_text);
                break;
            }
        }
    }

    bool CheckTree(const TreeNode& root) {
        if (root._val == -1) {
            return root._left && CheckTree(*root._left) && root._right && CheckTree(*root._right);
        } else {
            return root._left == nullptr && root._right == nullptr;
        }
    }

    unsigned char GetNextPlaintext(TreeNode& root, BitStream& bitstream) {
        const TreeNode* cur = &root;
        while (cur->_val == -1) {
            cur = bitstream.GetBit() ? cur->_right.get() : cur->_left.get();
        }
        return cur->_val;
    }

    void RealAssignHuffmanCoding(int* coding, unsigned char* code_length, const std::shared_ptr<TreeNode>& root, int cur_coding, unsigned char cur_code_length) {
        if (root->_val != -1) {
            coding[root->_val] = cur_coding;
            code_length[root->_val] = cur_code_length;
            return;
        }
        RealAssignHuffmanCoding(coding, code_length, root->_left, cur_coding << 1, cur_code_length + 1);
        RealAssignHuffmanCoding(coding, code_length, root->_right, (cur_coding << 1) + 1, cur_code_length + 1);
    }

    void AssignHuffmanCoding(int* coding, unsigned char* code_length, const std::shared_ptr<TreeNode>& root) {
        RealAssignHuffmanCoding(coding, code_length, root, 0, 0);
    }
}


void LosslessCompress::EntrophyEncoding(const std::vector<int>& data, std::stringstream& out) {
    int size = data.size();
    std::vector<unsigned char> lengths;
    BitStream bits_bitstream;
    int frequency[256] = {0};
    unsigned char uniform_length = 0;
    for (int i = 0; i < size; i++) {
        unsigned char length;
        int bits;
        GetLengthAndBits(data[i], length, bits);
        lengths.push_back(length);
        frequency[length]++;
        uniform_length = length;
        bits_bitstream.AddBits<int>(length, bits);
    }
    std::priority_queue<HuffmanCoding::Record> records;
    for (int i = 0; i < 256; i++) {
        if (frequency[i] > 0) {
            records.push({
                frequency[i], std::make_shared<HuffmanCoding::TreeNode>(
                    nullptr, nullptr, i
                )
            });
        }
    }
    while(records.size() > 1) {
        auto left = records.top();
        records.pop();
        auto right = records.top();
        records.pop();
        records.push(HuffmanCoding::Merge(left, right));
    }
    
    int coding[256];
    unsigned char code_length[256] = {0};
    HuffmanCoding::AssignHuffmanCoding(coding, code_length, records.top()._node);
    assert(HuffmanCoding::CheckTree(*records.top()._node));

    unsigned char useful_coding = 0;
    for (int i = 0; i < 256; i++) {
        if (code_length[i] > 0) {
            useful_coding++;
        }
    }
    // print huffman table
    out.write(reinterpret_cast<const char*>(&useful_coding), sizeof(useful_coding));
    if (useful_coding == 0) {
        out.write(reinterpret_cast<const char*>(&uniform_length), sizeof(uniform_length));
        out.write(reinterpret_cast<const char*>(&size), sizeof(size));
    } else {
        unsigned char plain_text = 0;
        do {
            if (code_length[plain_text] > 0) {
                out.write(reinterpret_cast<const char*>(&plain_text), sizeof(plain_text));
                out.write(reinterpret_cast<const char*>(&code_length[plain_text]), sizeof(code_length[plain_text]));
                out.write(reinterpret_cast<const char*>(&coding[plain_text]), sizeof(coding[plain_text]));
            }
            plain_text++;
        } while (plain_text);
        out.write(reinterpret_cast<const char*>(&size), sizeof(size));
        BitStream lengths_bitstream;
        for (int i = 0; i < size; i++) {
            lengths_bitstream.AddBits(code_length[lengths[i]], coding[lengths[i]]);
        }
        // spdlog::info("length storage space before compression: {}", lengths.size() * sizeof(unsigned char));
        // spdlog::info("length storage space: {}", lengths_bitstream.GetSize());
        out << lengths_bitstream;
    }
    // spdlog::info("bits storage space: {}", bits_bitstream.GetSize());
    out << bits_bitstream;
}

void LosslessCompress::EntrophyDecoding(std::stringstream& in, std::vector<int>& data) {
    unsigned char useful_coding;
    in.read(reinterpret_cast<char*>(&useful_coding), sizeof(useful_coding));
    int size;
    if (useful_coding == 0) {
        unsigned char unique_length;
        in.read(reinterpret_cast<char*>(&unique_length), sizeof(unique_length));
        in.read(reinterpret_cast<char*>(&size), sizeof(size));
        BitStream bits_bitstream;
        in >> bits_bitstream;
        for (int i = 0; i < size; i++) {
            int bits = bits_bitstream.GetBits<int>(unique_length);
            data.push_back(ReadLengthAndBits(unique_length, bits));
        }
    } else {
        HuffmanCoding::TreeNode root(nullptr, nullptr, -1);
        for (unsigned i = 0; i < useful_coding; i++) {
            unsigned char plain_text;
            unsigned char code_length;
            int code_text;
            in.read(reinterpret_cast<char*>(&plain_text), sizeof(plain_text));
            in.read(reinterpret_cast<char*>(&code_length), sizeof(code_length));
            in.read(reinterpret_cast<char*>(&code_text), sizeof(code_text));
            HuffmanCoding::AddCode(root, code_text, code_length, plain_text);
        }
        assert(HuffmanCoding::CheckTree(root));

        in.read(reinterpret_cast<char*>(&size), sizeof(size));
        std::vector<unsigned char> lengths;
        BitStream lengths_bitstream;
        in >> lengths_bitstream;
        for (int i = 0; i < size; i++) {
            lengths.push_back(HuffmanCoding::GetNextPlaintext(root, lengths_bitstream));
        }
        BitStream bits_bitstream;
        in >> bits_bitstream;
        for (int i = 0; i < size; i++) {
            int bits = bits_bitstream.GetBits<int>(lengths[i]);
            data.push_back(ReadLengthAndBits(lengths[i], bits));
        }
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
    // spdlog::info("Encoding diffs");
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
    // spdlog::info("Encoding zero counts");
    EntrophyEncoding(zero_counts, out);
    // spdlog::info("Encoding non-zero values");
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