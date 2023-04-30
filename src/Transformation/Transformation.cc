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
    block = (_dct * block.cast<double>() * _dct.transpose()).array().round().cast<int>();
}

void BasicDCT::InverseTransform(Ref<Matrix8i> block) const {
    block = (_dct.transpose() * block.cast<double>() * _dct).array().round().cast<int>();
}

const double FDCT::S[8] = {
    1.0 / (2 * sqrt(2)),
    1.0 / (4 * std::cos(pi * 1 / 16)),
    1.0 / (4 * std::cos(pi * 2 / 16)),
    1.0 / (4 * std::cos(pi * 3 / 16)),
    1.0 / (4 * std::cos(pi * 4 / 16)),
    1.0 / (4 * std::cos(pi * 5 / 16)),
    1.0 / (4 * std::cos(pi * 6 / 16)),
    1.0 / (4 * std::cos(pi * 7 / 16))
};

const double FDCT::A[6] = {
    NAN,
    std::cos(pi * 4 / 16),
    std::cos(pi * 2 / 16) - std::cos(pi * 6 / 16),
    std::cos(pi * 4 / 16),
    std::cos(pi * 2 / 16) + std::cos(pi * 6 / 16),
    std::cos(pi * 6 / 16),
};

// From https://web.stanford.edu/class/ee398a/handouts/lectures/07-TransformCoding.pdf#page=30
void FDCT::Transform(Ref<Matrix8i> block) const {
    Matrix8d block_double = block.cast<double>();
    for (int i = 0; i < 8; i++) {
        Transform1D(block_double.col(i).data(), block_double.rowStride());
    }

    for (int i = 0; i < 8; i++) {
        Transform1D(block_double.row(i).data(), block_double.colStride());
    }
    block = block_double.array().round().cast<int>();
}

void FDCT::InverseTransform(Ref<Matrix8i> block) const {
    Matrix8d block_double = block.cast<double>();
    for (int i = 0; i < 8; i++) {
        InverseTransform1D(block_double.row(i).data(), block_double.colStride());
    }
    for (int i = 0; i < 8; i++) {
        InverseTransform1D(block_double.col(i).data(), block_double.rowStride());
    }
    block = block_double.array().round().cast<int>();
}

void FDCT::Transform1D(double* vector, int stride) {
    int offset0 = 0;
    int offset1 = offset0 + stride;
    int offset2 = offset1 + stride;
    int offset3 = offset2 + stride;
    int offset4 = offset3 + stride;
    int offset5 = offset4 + stride;
    int offset6 = offset5 + stride;
    int offset7 = offset6 + stride;
    const double v0 = vector[offset0] + vector[offset7];
	const double v1 = vector[offset1] + vector[offset6];
	const double v2 = vector[offset2] + vector[offset5];
	const double v3 = vector[offset3] + vector[offset4];
	const double v4 = vector[offset3] - vector[offset4];
	const double v5 = vector[offset2] - vector[offset5];
	const double v6 = vector[offset1] - vector[offset6];
	const double v7 = vector[offset0] - vector[offset7];
	
	const double v8 = v0 + v3;
	const double v9 = v1 + v2;
	const double v10 = v1 - v2;
	const double v11 = v0 - v3;
	const double v12 = -v4 - v5;
	const double v13 = (v5 + v6) * A[3];
	const double v14 = v6 + v7;
	
	const double v15 = v8 + v9;
	const double v16 = v8 - v9;
	const double v17 = (v10 + v11) * A[1];
	const double v18 = (v12 + v14) * A[5];
	
	const double v19 = -v12 * A[2] - v18;
	const double v20 = v14 * A[4] - v18;
	
	const double v21 = v17 + v11;
	const double v22 = v11 - v17;
	const double v23 = v13 + v7;
	const double v24 = v7 - v13;
	
	const double v25 = v19 + v24;
	const double v26 = v23 + v20;
	const double v27 = v23 - v20;
	const double v28 = v24 - v19;
	
	vector[offset0] = S[0] * v15;
	vector[offset1] = S[1] * v26;
	vector[offset2] = S[2] * v21;
	vector[offset3] = S[3] * v28;
	vector[offset4] = S[4] * v16;
	vector[offset5] = S[5] * v25;
	vector[offset6] = S[6] * v22;
	vector[offset7] = S[7] * v27;
}

void FDCT::InverseTransform1D(double* vector, int stride) {
    int offset0 = 0;
    int offset1 = offset0 + stride;
    int offset2 = offset1 + stride;
    int offset3 = offset2 + stride;
    int offset4 = offset3 + stride;
    int offset5 = offset4 + stride;
    int offset6 = offset5 + stride;
    int offset7 = offset6 + stride;
    const double v15 = vector[offset0] / S[0];
	const double v26 = vector[offset1] / S[1];
	const double v21 = vector[offset2] / S[2];
	const double v28 = vector[offset3] / S[3];
	const double v16 = vector[offset4] / S[4];
	const double v25 = vector[offset5] / S[5];
	const double v22 = vector[offset6] / S[6];
	const double v27 = vector[offset7] / S[7];
	
	const double v19 = (v25 - v28) / 2;
	const double v20 = (v26 - v27) / 2;
	const double v23 = (v26 + v27) / 2;
	const double v24 = (v25 + v28) / 2;
	
	const double v7  = (v23 + v24) / 2;
	const double v11 = (v21 + v22) / 2;
	const double v13 = (v23 - v24) / 2;
	const double v17 = (v21 - v22) / 2;
	
	const double v8 = (v15 + v16) / 2;
	const double v9 = (v15 - v16) / 2;
	
	const double v18 = (v19 - v20) * A[5];  // Different from original
	const double v12 = (v19 * A[4] - v18) / (A[2] * A[5] - A[2] * A[4] - A[4] * A[5]);
	const double v14 = (v18 - v20 * A[2]) / (A[2] * A[5] - A[2] * A[4] - A[4] * A[5]);
	
	const double v6 = v14 - v7;
	const double v5 = v13 / A[3] - v6;
	const double v4 = -v5 - v12;
	const double v10 = v17 / A[1] - v11;
	
	const double v0 = (v8 + v11) / 2;
	const double v1 = (v9 + v10) / 2;
	const double v2 = (v9 - v10) / 2;
	const double v3 = (v8 - v11) / 2;

	vector[offset0] = (v0 + v7) / 2;
	vector[offset1] = (v1 + v6) / 2;
	vector[offset2] = (v2 + v5) / 2;
	vector[offset3] = (v3 + v4) / 2;
	vector[offset4] = (v3 - v4) / 2;
	vector[offset5] = (v2 - v5) / 2;
	vector[offset6] = (v1 - v6) / 2;
	vector[offset7] = (v0 - v7) / 2;
}

Transformation* TransformationFactory::GetTransformation(const std::string& type) {
    if (type == "basic") {
        return new BasicDCT;
    } else if (type == "FDCT") {
        return new FDCT;
    } else {
        throw std::logic_error("Unimplemented transformation");
    }
}