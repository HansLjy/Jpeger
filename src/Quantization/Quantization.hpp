#pragma once

#include "Common.hpp"

class Quantization {
public:
    virtual void QuantizeY(Ref<MatrixXi> mat) const = 0;
    virtual void QuantizeU(Ref<MatrixXi> mat) const = 0;
    virtual void QuantizeV(Ref<MatrixXi> mat) const = 0;

    virtual void InverseQuantizeY(Ref<MatrixXi> mat) const = 0;
    virtual void InverseQuantizeU(Ref<MatrixXi> mat) const = 0;
    virtual void InverseQuantizeV(Ref<MatrixXi> mat) const = 0;

};

class JpegQuantization : public Quantization {
public:
    void QuantizeY(Ref<MatrixXi> mat) const override;
    void QuantizeU(Ref<MatrixXi> mat) const override;
    void QuantizeV(Ref<MatrixXi> mat) const override;

    void InverseQuantizeY(Ref<MatrixXi> mat) const override;
    void InverseQuantizeU(Ref<MatrixXi> mat) const override;
    void InverseQuantizeV(Ref<MatrixXi> mat) const override;

protected:
    static const int _block_size = 8;
    static const Matrix8i _Y_quantization_table;
    static const Matrix8i _UV_quantization_table;
};

class NullQuantization : public Quantization {
public:
    void QuantizeY(Ref<MatrixXi> mat) const override {};
    void QuantizeU(Ref<MatrixXi> mat) const override {};
    void QuantizeV(Ref<MatrixXi> mat) const override {};

    void InverseQuantizeY(Ref<MatrixXi> mat) const override {};
    void InverseQuantizeU(Ref<MatrixXi> mat) const override {};
    void InverseQuantizeV(Ref<MatrixXi> mat) const override {};
};

class QuantizationFactory {
public:
    static Quantization* GetQuantization(const std::string& type);
};
