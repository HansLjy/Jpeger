#include "gtest/gtest.h"
#include "Color.hpp"

int rows = 10, cols = 10;

TEST(ColorTest, YUVTest) {
    srand(0);
    MatrixXi R(rows, cols), G(rows, cols), B(rows, cols);
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            R(row, col) = rand() % 256;
            G(row, col) = rand() % 256;
            B(row, col) = rand() % 256;
        }
    }
    RGB rgb{R, G, B};
    auto rgb_result = RGB(YUV(rgb));
    // EXPECT_EQ(rgb._R, rgb_result._R);
    // EXPECT_EQ(rgb._G, rgb_result._G);
    // EXPECT_EQ(rgb._B, rgb_result._B);
    std::cerr << rgb._R - rgb_result._R << std::endl;
    std::cerr << rgb._G - rgb_result._G << std::endl;
    std::cerr << rgb._B - rgb_result._B << std::endl;
}