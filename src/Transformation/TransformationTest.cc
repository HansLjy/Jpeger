#include "gtest/gtest.h"
#include "Transformation.hpp"

TEST(Transformation, BasicDCTTest) {
    srand(0);
    BasicDCT basic_dct;
    Matrix8i block;
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            block(row, col) = rand() % 256;
        }
    }
    Matrix8i block_result = block;
    basic_dct.Transform(block_result);
    basic_dct.InverseTransform(block_result);
    // EXPECT_EQ(block_result, block);
    std::cerr << block_result - block;
}


TEST(Transformation, FDCTTest) {
    srand(0);
    FDCT fdct;
    Matrix8i block;
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            block(row, col) = rand() % 256;
        }
    }
    Matrix8i block_result = block;
    fdct.Transform(block_result);
    fdct.InverseTransform(block_result);
    // EXPECT_EQ(block_result, block);
    std::cerr << block_result - block;
}