#include "ImageIO.hpp"
#include "libbmp.h"

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