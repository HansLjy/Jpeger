#include "ImageIO.hpp"
#include "libbmp.h"
#include <fstream>

RGB BMPIO::Read(const std::string &filename) const {
    BmpImg img;
    img.read(filename);
    
    const int total_rows = img.get_height();
    const int total_cols = img.get_width();
    MatrixXi R(total_rows, total_cols), G(total_rows, total_cols), B(total_rows, total_cols);

    for (int col = 0; col < total_cols; col++) {
        for (int row = 0; row < total_rows; row++) {
            R(row, col) = img.red_at(col, row);
            G(row, col) = img.green_at(col, row);
            B(row, col) = img.blue_at(col, row);
        }
    }
    return {R, G, B};
}

void BMPIO::Write(const RGB &rgb, const std::string &filename) const {
    const int total_rows = rgb._R.rows();
    const int total_cols = rgb._R.cols();
    BmpImg img(total_cols, total_rows);
    
    for (int col = 0; col < total_cols; col++) {
        for (int row = 0; row < total_rows; row++) {
            img.set_pixel(col, row, rgb._R(row, col), rgb._G(row, col), rgb._B(row, col));
        }
    }

    img.write(filename);
}

RGB PPMIO::Read(const std::string &filename) const {
    std::fstream ppm(filename);
    std::string title;
    ppm >> title;
    int rows, cols;
    ppm >> cols >> rows;
    int type;
    ppm >> type;
    std::string tmp;
    getline(ppm, tmp);
    int padded_rows = ((rows + 15) >> 4) << 4;
    int padded_cols = ((cols + 15) >> 4) << 4;
    RGB rgb{
        MatrixXi::Zero(padded_rows, padded_cols),
        MatrixXi::Zero(padded_rows, padded_cols),
        MatrixXi::Zero(padded_rows, padded_cols)
    };
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            unsigned char r, g, b;
            ppm.read(reinterpret_cast<char*>(&r), sizeof(r));
            ppm.read(reinterpret_cast<char*>(&g), sizeof(g));
            ppm.read(reinterpret_cast<char*>(&b), sizeof(b));
            rgb._R(row, col) = r;
            rgb._G(row, col) = g;
            rgb._B(row, col) = b;
        }
    }

    return rgb;
}

void PPMIO::Write(const RGB &rgb, const std::string &filename) const {
    throw std::logic_error("Unimplemented error");
}