#include "Jpeger.hpp"
#include <fstream>

Jpeger::Jpeger(const Json::Value& config)
    : _quantizer(QuantizationFactory::GetQuantization(config["Quantization"].asString())),
      _transformation(TransformationFactory::GetTransformation(config["Transformation"].asString())),
      _DC_compression(LosslessCompressFactory<int>::GetLosslessCompress(config["DC-compress"].asString())),
      _AC_compression(LosslessCompressFactory<int>::GetLosslessCompress(config["AC-compress"].asString())){}

void Jpeger::Compress(const RGB &data, const std::string &filename) const {
    RGB origin = data;
    auto yuv = YUV(data);

    const int total_rows = data._R.rows();
    const int total_cols = data._R.cols();
    const int down_sampled_rows = total_rows >> 1;
    const int down_sampled_cols = total_cols >> 1;
    
    MatrixXi Y = yuv._Y, U(down_sampled_rows, down_sampled_cols), V(down_sampled_rows, down_sampled_cols);

    // subsampling
    for (int row = 0; row < total_rows; row += 2) {
        for (int col = 0; col < total_cols; col += 2) {
            U(row >> 1, col >> 1) = (yuv._U(row, col) + yuv._U(row + 1, col) + yuv._U(row, col + 1) + yuv._U(row + 1, col + 1)) >> 2;
            V(row >> 1, col >> 1) = (yuv._V(row, col) + yuv._V(row + 1, col) + yuv._V(row, col + 1) + yuv._V(row + 1, col + 1)) >> 2;
        }
    }

    // DCT
    for (int row = 0; row < total_rows; row += _block_size) {
        for (int col = 0; col < total_cols; col += _block_size) {
            _transformation->Transform(Y.block<_block_size, _block_size>(row, col));
        }
    }
    for (int row = 0; row < down_sampled_rows; row += _block_size) {
        for (int col = 0; col < down_sampled_cols; col += _block_size) {
            _transformation->Transform(U.block<_block_size, _block_size>(row, col));
            _transformation->Transform(V.block<_block_size, _block_size>(row, col));
        }
    }

    _quantizer->QuantizeY(Y);
    _quantizer->QuantizeU(U);
    _quantizer->QuantizeV(V);

    VectorXi Y_DC(total_cols * total_rows / (_block_size * _block_size));
    VectorXi Y_AC(total_cols * total_rows - total_cols * total_rows / (_block_size * _block_size));
    int Y_DC_index = 0;
    int Y_AC_index = 0;
    for (int col = 0; col < total_cols; col += _block_size) {
        for (int row = 0; row < total_rows; row += _block_size) {
            Y_DC(Y_DC_index++) = Y(row, col);
            ZigZag(Y.block<_block_size, _block_size>(row, col), Y_AC.segment<_block_size * _block_size - 1>(Y_AC_index));
            Y_AC_index += _block_size * _block_size - 1;
        }
    }
    
    VectorXi U_DC(down_sampled_cols * down_sampled_rows / (_block_size * _block_size));
    VectorXi U_AC(down_sampled_cols * down_sampled_rows - down_sampled_cols * down_sampled_rows / (_block_size * _block_size));
    VectorXi V_DC(U_DC.size());
    VectorXi V_AC(U_AC.size());
    int UV_DC_index = 0;
    int UV_AC_index = 0;
    for (int col = 0; col < down_sampled_cols; col += _block_size) {
        for (int row = 0; row < down_sampled_rows; row += _block_size) {
            U_DC(UV_DC_index) = U(row, col);
            V_DC(UV_DC_index) = V(row, col);
            ZigZag(U.block<_block_size, _block_size>(row, col), U_AC.segment<_block_size * _block_size - 1>(UV_AC_index));
            ZigZag(V.block<_block_size, _block_size>(row, col), V_AC.segment<_block_size * _block_size - 1>(UV_AC_index));
            UV_DC_index++;
            UV_AC_index += _block_size * _block_size - 1;
        }
    }

    std::stringstream output;
    output.write(reinterpret_cast<const char*>(&total_rows), sizeof(int));
    output.write(reinterpret_cast<const char*>(&total_cols), sizeof(int));
    _DC_compression->Compress(Y_DC, output);
    _DC_compression->Compress(U_DC, output);
    _DC_compression->Compress(V_DC, output);
    _AC_compression->Compress(Y_AC, output);
    _AC_compression->Compress(U_AC, output);
    _AC_compression->Compress(V_AC, output);
    
    std::ofstream output_file(filename);
    output_file << output.rdbuf();
    output_file.close();
}


RGB Jpeger::Decompress(const std::string &filename) const {
    std::ifstream input_file(filename);
    std::stringstream input;
    input << input_file.rdbuf();

    int total_rows, total_cols;
    input.read(reinterpret_cast<char*>(&total_rows), sizeof(int));
    input.read(reinterpret_cast<char*>(&total_cols), sizeof(int));

    const int down_sampled_rows = total_rows >> 1;
    const int down_sampled_cols = total_cols >> 1;

    VectorXi Y_DC(total_cols * total_rows / (_block_size * _block_size));
    VectorXi Y_AC(total_cols * total_rows - total_cols * total_rows / (_block_size * _block_size));
    VectorXi U_DC(down_sampled_cols * down_sampled_rows / (_block_size * _block_size));
    VectorXi U_AC(down_sampled_cols * down_sampled_rows - down_sampled_cols * down_sampled_rows / (_block_size * _block_size));
    VectorXi V_DC(U_DC.size());
    VectorXi V_AC(U_AC.size());

    _DC_compression->InverseCompress(input, Y_DC);
    _DC_compression->InverseCompress(input, U_DC);
    _DC_compression->InverseCompress(input, V_DC);

    _AC_compression->InverseCompress(input, Y_AC);
    _AC_compression->InverseCompress(input, U_AC);
    _AC_compression->InverseCompress(input, V_AC);

    MatrixXi Y(total_rows, total_cols);
    MatrixXi U(down_sampled_rows, down_sampled_cols);
    MatrixXi V(down_sampled_rows, down_sampled_cols);

    int UV_DC_index = 0;
    int UV_AC_index = 0;
    for (int col = 0; col < down_sampled_cols; col += _block_size) {
        for (int row = 0; row < down_sampled_rows; row += _block_size) {
            U(row, col) = U_DC(UV_DC_index);
            V(row, col) = V_DC(UV_DC_index);
            InverseZigZag(U_AC.segment<_block_size * _block_size - 1>(UV_AC_index), U.block<_block_size, _block_size>(row, col));
            InverseZigZag(V_AC.segment<_block_size * _block_size - 1>(UV_AC_index), V.block<_block_size, _block_size>(row, col));
            UV_DC_index++;
            UV_AC_index += _block_size * _block_size - 1;
        }
    }

    int Y_DC_index = 0;
    int Y_AC_index = 0;
    for (int col = 0; col < total_cols; col += _block_size) {
        for (int row = 0; row < total_rows; row += _block_size) {
            Y(row, col) = Y_DC(Y_DC_index);
            InverseZigZag(Y_AC.segment<_block_size * _block_size - 1>(Y_AC_index), Y.block<_block_size, _block_size>(row, col));
            Y_DC_index++;
            Y_AC_index += _block_size * _block_size - 1;
        }
    }

    _quantizer->InverseQuantizeY(Y);
    _quantizer->InverseQuantizeU(U);
    _quantizer->InverseQuantizeV(V);

    for (int col = 0; col < total_cols; col += _block_size) {
        for (int row = 0; row < total_rows; row += _block_size) {
            _transformation->InverseTransform(Y.block<_block_size, _block_size>(row, col));
        }
    }
    for (int col = 0; col < down_sampled_cols; col += _block_size) {
        for (int row = 0; row < down_sampled_rows; row += _block_size) {
            _transformation->InverseTransform(U.block<_block_size, _block_size>(row, col));
            _transformation->InverseTransform(V.block<_block_size, _block_size>(row, col));
        }
    }

    MatrixXi U_full(total_rows, total_cols), V_full(total_rows, total_cols);

    for (int col = 0; col < total_cols; col += 2) {
        for (int row = 0; row < total_rows; row += 2) {
            U_full(row, col) = U_full(row, col + 1) = U_full(row + 1, col) = U_full(row + 1, col + 1) = U(row >> 1, col >> 1);
            V_full(row, col) = V_full(row, col + 1) = V_full(row + 1, col) = V_full(row + 1, col + 1) = V(row >> 1, col >> 1);
        }
    }

    return RGB(YUV(Y, U_full, V_full));
}

void Jpeger::ZigZag(const Ref<MatrixXi>& block, Ref<VectorXi> result) {
    int index = 0;
    for (int sum = 1; sum < _block_size * 2 - 1; sum++) {
        if (sum & 1) {
            // right to left
            int row = (sum < _block_size ? 0 : sum - _block_size + 1);
            int col = (sum < _block_size ? sum : _block_size - 1);
            for (; row < _block_size && col >= 0; row++, col--) {
                result(index++) = block(row, col);
            }
        } else {
            // left to right
            int row = (sum < _block_size ? sum : _block_size - 1);
            int col = (sum < _block_size ? 0 : sum - _block_size + 1);
            for (; row >= 0 && col < _block_size; row--, col++) {
                result(index++) = block(row, col);
            }
        }
    }
}

void Jpeger::InverseZigZag(const Ref<VectorXi>& vec, Ref<MatrixXi> block) {
    int index = 0;
    for (int sum = 1; sum < _block_size * 2 - 1; sum++) {
        if (sum & 1) {
            // right to left
            int row = (sum < _block_size ? 0 : sum - _block_size + 1);
            int col = (sum < _block_size ? sum : _block_size - 1);
            for (; row < _block_size && col >= 0; row++, col--) {
                block(row, col) = vec(index++);
            }
        } else {
            // left to right
            int row = (sum < _block_size ? sum : _block_size - 1);
            int col = (sum < _block_size ? 0 : sum - _block_size + 1);
            for (; row >= 0 && col < _block_size; row--, col++) {
                block(row, col) = vec(index++);
            }
        }
    }
}

