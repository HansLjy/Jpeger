#include "Color.hpp"

const Matrix<double, 3, 3> YUV::_rgb2yuv = (Matrix<double, 3, 3>() << 
    0.299,      0.587,      0.114,
    -0.14713,   -0.28886,   0.436,
    0.615,      -0.51499,   -0.10001
).finished();

const Matrix<double, 3, 3> YUV::_yuv2rgb = (Matrix<double, 3, 3>() << 
    1,      0,          1.13983,
    1,      -0.39465,   -0.58060,
    1,      2.03211,    0
).finished();

YUV::YUV(const MatrixXi& Y, const MatrixXi& U, const MatrixXi& V) : _Y(Y), _U(U), _V(V) {}

YUV::YUV(const RGB& rgb)
    : _Y(rgb._R.rows(), rgb._R.cols()),
      _U(rgb._R.rows(), rgb._R.cols()),
      _V(rgb._R.rows(), rgb._R.cols()) {
    const int total_rows = rgb._R.rows();
    const int total_cols = rgb._R.cols();
    for (int col = 0; col < total_cols; col++) {
        for (int row = 0; row < total_rows; row++) {
            Eigen::Vector3d result = _rgb2yuv * (Eigen::Vector3d() << 
                rgb._R(row, col), rgb._G(row, col), rgb._B(row, col)
            ).finished();
            _Y(row, col) = result(0);
            _U(row, col) = result(1);
            _V(row, col) = result(2);
        }
    }
}

YUV::operator RGB() {
    const int total_rows = _Y.rows();
    const int total_cols = _Y.cols();
    MatrixXi R(total_rows, total_cols), G(total_rows, total_cols), B(total_rows, total_cols);
    for (int col = 0; col < total_cols; col++) {
        for (int row = 0; row < total_rows; row++) {
            Eigen::Vector3d result = _yuv2rgb * (Eigen::Vector3d() << 
                _Y(row, col), _U(row, col), _V(row, col)
            ).finished();
            R(row, col) = result(0);
            G(row, col) = result(1);
            B(row, col) = result(2);
        }
    }
    return {R, G, B};
}