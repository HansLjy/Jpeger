#pragma once

#include "Common.hpp"

class Transformation {
public:
    virtual void Transform(Ref<Matrix8i> block) const = 0;
    virtual void InverseTransform(Ref<Matrix8i> block) const = 0;
};

class BasicDCT : public Transformation {
public:
    void Transform(Ref<Matrix8i> block) const override;
    void InverseTransform(Ref<Matrix8i> block) const override;

protected:
    static const int _block_size;
    static Matrix8d GetDCTMatrix();
    static const Matrix8d _dct;
};

class TransformationFactory {
public:
    static Transformation* GetTransformation(const std::string& type);
};