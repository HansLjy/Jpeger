#pragma once

#include "Common.hpp"

struct RGB {
    MatrixXi _R, _G, _B;
};

struct YUV {
    MatrixXi _Y, _U, _V;
    explicit YUV(const MatrixXi& Y, const MatrixXi& U, const MatrixXi& V);
    explicit YUV(const RGB& rgb);
    explicit operator RGB();

protected:
    static const Matrix<double, 3, 3> _rgb2yuv;
    static const Matrix<double, 3, 3> _yuv2rgb;
};