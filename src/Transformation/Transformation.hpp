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

class FDCT : public Transformation {
public:
    void Transform(Ref<Matrix8i> block) const override;
    void InverseTransform(Ref<Matrix8i> block) const override;

protected:
    static void Transform1D(double* vector, int stride);
    static void InverseTransform1D(double* vector, int stride);
    static const double S[8];
    static const double A[6];
};

class BinDCT : public Transformation {
public:
    void Transform(Ref<Matrix8i> block) const override;
    void InverseTransform(Ref<Matrix8i> block) const override;

protected:
    static void Transform1D(int* vector, int stride);
    static void InverseTransform1D(int* vector, int stride);
    static const double S[8];
};

class NullDCT : public Transformation {
public:
    void Transform(Ref<Matrix8i> block) const override {};
    void InverseTransform(Ref<Matrix8i> block) const override {};
};

class TransformationFactory {
public:
    static Transformation* GetTransformation(const std::string& type);
};