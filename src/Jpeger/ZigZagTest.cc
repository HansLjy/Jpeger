#include "Jpeger.hpp"
#include "gtest/gtest.h"

class FriendlyJpeger : public Jpeger {
public:
    static void FriendlyZigZag(const Ref<MatrixXi>& block, Ref<VectorXi> result) {
        Jpeger::ZigZag(block, result);
    }
    static void InverseZigZag(const Ref<VectorXi>& vec, Ref<MatrixXi> block) {
        Jpeger::InverseZigZag(vec, block);
    }
};

TEST(Jpeger, ZigZagTest) {
    Matrix8i block = Matrix8i::Random(), block_result;
    VectorXi result(63);
    FriendlyJpeger::FriendlyZigZag(block, result);
    FriendlyJpeger::InverseZigZag(result, block_result);
    block_result(0, 0) = block(0, 0);

    EXPECT_EQ(block, block_result);
}