#include "Transformation.hpp"
#include <cmath>

const double pi = acos(-1);

Matrix8d BasicDCT::GetDCTMatrix() {
    Matrix8d dct;
    for (int k = 0; k < _block_size; k++) {
        double C = k == 0 ? 1.0 / sqrt(_block_size) : sqrt(2.0 / _block_size);
        for (int j = 0; j < _block_size; j++) {
            dct(k, j) = C * std::cos((double)(2 * j + 1) / (2 * _block_size) * k * pi);
        }
    }
    assert((dct * dct.transpose() - Matrix8d::Identity()).norm() < 1e-10);
    return dct;
};

const Matrix8d BasicDCT::_dct = GetDCTMatrix();
const int BasicDCT::_block_size = 8;

void BasicDCT::Transform(Ref<Matrix8i> block) const {
    block = (_dct * block.cast<double>() * _dct.transpose()).cast<int>();
}

void BasicDCT::InverseTransform(Ref<Matrix8i> block) const {
    block = (_dct.transpose() * block.cast<double>() * _dct).cast<int>();
}

Transformation* TransformationFactory::GetTransformation(const std::string& type) {
    if (type == "basic") {
        return new BasicDCT;
    } else {
        throw std::logic_error("Unimplemented transformation");
    }
}