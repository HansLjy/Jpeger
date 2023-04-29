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
    : _Y((rgb._R.array().cast<float>() * _rgb2yuv(0, 0) + rgb._G.array().cast<float>() * _rgb2yuv(0, 1) + rgb._B.array().cast<float>() * _rgb2yuv(0, 2)).cast<int>()),
      _U((rgb._R.array().cast<float>() * _rgb2yuv(1, 0) + rgb._G.array().cast<float>() * _rgb2yuv(1, 1) + rgb._B.array().cast<float>() * _rgb2yuv(1, 2)).cast<int>()),
      _V((rgb._R.array().cast<float>() * _rgb2yuv(2, 0) + rgb._G.array().cast<float>() * _rgb2yuv(2, 1) + rgb._B.array().cast<float>() * _rgb2yuv(2, 2)).cast<int>()) {
}

YUV::operator RGB() {
    return {
        (_Y.array().cast<float>() * _yuv2rgb(0, 0) + _U.array().cast<float>() * _yuv2rgb(0, 1) + _V.array().cast<float>() * _yuv2rgb(0, 2)).cast<int>(),
        (_Y.array().cast<float>() * _yuv2rgb(1, 0) + _U.array().cast<float>() * _yuv2rgb(1, 1) + _V.array().cast<float>() * _yuv2rgb(1, 2)).cast<int>(),
        (_Y.array().cast<float>() * _yuv2rgb(2, 0) + _U.array().cast<float>() * _yuv2rgb(2, 1) + _V.array().cast<float>() * _yuv2rgb(2, 2)).cast<int>(),
    };
}