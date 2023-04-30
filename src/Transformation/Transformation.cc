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

const double BinDCT::S[8] = {
	std::sin(pi / 4) / 2,
	std::sin(pi / 4),
	std::sin(3 * pi / 8) / 2,
	1.0 / (2 * std::sin(3 * pi / 8)),
	std::sin(7 * pi / 16) / 2,
	std::cos(3 * pi / 16) / 2,
	1.0 / (2 * std::cos(3 * pi / 16)),
	1.0 / (2 * std::sin(7 * pi / 16))
};

void BinDCT::Transform(Ref<Matrix8i> block) const {
    for (int i = 0; i < 8; i++) {
        Transform1D(block.col(i).data(), block.rowStride());
    }
    for (int i = 0; i < 8; i++) {
        Transform1D(block.row(i).data(), block.colStride());
    }
}

void BinDCT::InverseTransform(Ref<Matrix8i> block) const {
    for (int i = 0; i < 8; i++) {
        InverseTransform1D(block.row(i).data(), block.colStride());
    }
    for (int i = 0; i < 8; i++) {
        InverseTransform1D(block.col(i).data(), block.rowStride());
    }
}

#define PROD3(x)  (x + (x << 1))
#define PROD11(x) (x + (x << 1) + (x << 3))
#define PROD13(x) (x + (x << 2) + (x << 3))
#define PROD15(x) (x + (x << 1) + (x << 2) + (x << 3))
#define P1(x) (PROD13(x) >> 5)
#define U1(x) (PROD11(x) >> 5)
#define P2(x) (PROD11(x) >> 4)
#define U2(x) (PROD15(x) >> 5)
#define P3(x) (PROD3(x) >> 4)
#define U3(x) (PROD3(x) >> 4)
#define P4(x) (PROD13(x) >> 5)
#define U4(x) (PROD11(x) >> 4)
#define P5(x) (PROD13(x) >> 5)

void BinDCT::Transform1D(int* vector, int stride) {
	int offset0 = 0;
    int offset1 = offset0 + stride;
    int offset2 = offset1 + stride;
    int offset3 = offset2 + stride;
    int offset4 = offset3 + stride;
    int offset5 = offset4 + stride;
    int offset6 = offset5 + stride;
    int offset7 = offset6 + stride;
    int v0 = vector[offset0] + vector[offset7];
	int v1 = vector[offset1] + vector[offset6];
	int v2 = vector[offset2] + vector[offset5];
	int v3 = vector[offset3] + vector[offset4];
	int v4 = vector[offset3] - vector[offset4];
	int v5 = vector[offset2] - vector[offset5];
	int v6 = vector[offset1] - vector[offset6];
	int v7 = vector[offset0] - vector[offset7];

	int v8  = v0 + v3;
	int v9  = v1 + v2;
	int v10 = v1 - v2;
	int v11 = v0 - v3;
	v8 = v8 + v9;
	v9 = (v8 >> 1) - v9;
	v10 = P1(v11) - v10;
	v11 = v11 - U1(v10);

	v5 = v5 - P4(v6);
	v6 = v6 + U4(v5);
	v5 = P5(v6) - v5;
	int v12 = v4 + v5;
	int v13 = v4 - v5;
	int v14 = v7 - v6;
	int v15 = v6 + v7;
	v12 = P3(v15) - v12;
	v15 = v15 - U3(v12);
	v13 = v13 + P2(v14);
	v14 = v14 - U2(v13);

	vector[offset0] = v8 * S[0];
	vector[offset4] = v9 * S[1];
	vector[offset6] = v10 * S[2];
	vector[offset2] = v11 * S[3];
	vector[offset7] = v12 * S[4];
	vector[offset5] = v13 * S[5];
	vector[offset3] = v14 * S[6];
	vector[offset1] = v15 * S[7];
}

void BinDCT::InverseTransform1D(int* vector, int stride) {
	int offset0 = 0;
    int offset1 = offset0 + stride;
    int offset2 = offset1 + stride;
    int offset3 = offset2 + stride;
    int offset4 = offset3 + stride;
    int offset5 = offset4 + stride;
    int offset6 = offset5 + stride;
    int offset7 = offset6 + stride;
	
	int v0 = vector[offset0] / S[0];
	int v1 = vector[offset4] / S[1];
	int v2 = vector[offset6] / S[2];
	int v3 = vector[offset2] / S[3];
	int v4 = vector[offset7] / S[4];
	int v5 = vector[offset5] / S[5];
	int v6 = vector[offset3] / S[6];
	int v7 = vector[offset1] / S[7];

	v1 = (v0 >> 1) - v1;
	v0 = v0 - v1;
	v3 = v3 + U1(v2);
	v2 = P1(v3) - v2;
	
	v6 = v6 + U2(v5);
	v5 = v5 - P2(v6);
	v7 = v7 + U3(v4);
	v4 = P3(v7) - v4;

	int v8 = v0 + v3;
	int v11 = v0 - v3;
	int v9 = v1 + v2;
	int v10 = v1 - v2;
	int v12 = v4 + v5;
	int v13 = v4 - v5;
	int v14 = v7 - v6;
	int v15 = v6 + v7;
	v13 = P5(v14) - v13;
	v14 = v14 - U4(v13);
	v13 = v13 + P4(v14);

	vector[offset0] = (v8 + v15) >> 2;
	vector[offset1] = (v9 + v14) >> 2;
	vector[offset2] = (v10 + v13) >> 2;
	vector[offset3] = (v11 + v12) >> 2;
	vector[offset4] = (v11 - v12) >> 2;
	vector[offset5] = (v10 - v13) >> 2;
	vector[offset6] = (v9 - v14) >> 2;
	vector[offset7] = (v8 - v15) >> 2;
}


Transformation* TransformationFactory::GetTransformation(const std::string& type) {
    if (type == "basic") {
        return new BasicDCT;
    } else if (type == "FDCT") {
        return new FDCT;
	} else if (type == "BDCT") {
		return new BinDCT;
    } else {
        throw std::logic_error("Unimplemented transformation");
    }
}