#include "gtest/gtest.h"
#include <vector>
#include "LosslessCompress.hpp"

const int vec_length = 10;

TEST(LosslessCompress, BitStreamTest) {
    srand(0);
    std::vector<int> vec;
    std::vector<unsigned char> lengths;
    for (int i = 0; i < vec_length; i++) {
        unsigned char length = rand() % 20;
        vec.push_back(rand() & ((1 << length) - 1));
        lengths.push_back(length);
    }
    BitStream input;
    for (int i = 0; i < vec_length; i++) {
        input.AddBits(lengths[i], vec[i]);
    }
    std::stringstream strstrm;
    strstrm << input;
    BitStream output;
    strstrm >> output;
    int position = 0;
    for (int i = 0; i < vec_length; i++) {
        int val = output.GetBits<int>(position, lengths[i]);
        position += lengths[i];
        EXPECT_EQ(val, vec[i]);
    }
}

TEST(LosslessCompress, DPCMTest) {
    DPCM<int> dpcm;
    VectorXi vec = VectorXi::Random(vec_length);
    for (int i = 0; i < vec_length; i++) {
        vec(i) &= 255;
    }
    VectorXi vec_decompressed(vec_length);
    std::stringstream strstrm;
    dpcm.Compress(vec, strstrm);
    dpcm.InverseCompress(strstrm, vec_decompressed);
    EXPECT_EQ(vec_decompressed, vec);
}

TEST(LosslessCompress, RLCTest) {
    srand(0);

    VectorXi vals = VectorXi::Zero(vec_length);

    for (int i = 0; i < vec_length; i++) {
        int coin = rand() % 3;
        if (coin > 1) {
            vals(i) = rand() % 10;
        }
    }
    
    RLC<int> rlc;
    std::stringstream strstrm;
    VectorXi vals_decompressed(vec_length);
    rlc.Compress(vals, strstrm);
    rlc.InverseCompress(strstrm, vals_decompressed);
    EXPECT_EQ(vals_decompressed, vals);

    // special case: all zero:

    vals.setZero();
    strstrm.clear();
    rlc.Compress(vals, strstrm);
    rlc.InverseCompress(strstrm, vals_decompressed);
    EXPECT_EQ(vals_decompressed, vals);
}