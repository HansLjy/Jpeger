#pragma once

#include <Eigen/Dense>
#include <iostream>
#include <vector>
#include "Common.hpp"
#include "Color.hpp"
#include "Quantization.hpp"
#include "Transformation.hpp"
#include "LosslessCompress.hpp"
#include "jsoncpp/json/json.h"

class Jpeger {
public:
    Jpeger(const Json::Value& config);
    void Compress(const RGB& data, const std::string& filename) const;
    RGB Decompress(const std::string& filename) const;

protected:
    static const int _block_size = 8;
    Quantization* _quantizer;
    Transformation* _transformation;
    LosslessCompress* _DC_compression;
    LosslessCompress* _AC_compression;

    static const int _indices[64];
    static void ZigZag(const Ref<MatrixXi>& block, Ref<VectorXi> result);
    static void InverseZigZag(const Ref<VectorXi>& vec, Ref<MatrixXi> block);
};