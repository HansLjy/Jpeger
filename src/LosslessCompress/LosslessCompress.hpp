#pragma once

#include <vector>
#include "Common.hpp"
#include "BitStream.hpp"
#include "spdlog/spdlog.h"

class LosslessCompress {
public:
    virtual void Compress(const Ref<VectorXi>& vector, std::stringstream& out) const = 0;
    virtual void InverseCompress(std::stringstream& in, Ref<VectorXi> vector) const = 0;

    static void EntrophyEncoding(const std::vector<int>& data, std::stringstream& out);
    static void EntrophyDecoding(std::stringstream& in, std::vector<int>& data);

protected:
    static void GetLengthAndBits(int x, unsigned char& length, int& bits);
    static int ReadLengthAndBits(unsigned char length, int bits);
};

class DPCM : public LosslessCompress {
public:
    void Compress(const Ref<VectorXi> &vector, std::stringstream &out) const override;
    void InverseCompress(std::stringstream &in, Ref<VectorXi> vector) const override;
};

class RLC : public LosslessCompress{
public:
    void Compress(const Ref<VectorXi> &vector, std::stringstream &out) const override;
    void InverseCompress(std::stringstream &in, Ref<VectorXi> vector) const override;
};

class LosslessCompressFactory {
public:
    static LosslessCompress* GetLosslessCompress(const std::string& type);
};


