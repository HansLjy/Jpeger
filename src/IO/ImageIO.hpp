#pragma once

#include <string>
#include "Color.hpp"

class ImageIO {
public:
    virtual RGB Read(const std::string& filename) const = 0;
    virtual void Write(const RGB& rgb, const std::string& filename) const = 0;
};

class BMPIO : public ImageIO {
public:
    RGB Read(const std::string &filename) const override;
    void Write(const RGB &rgb, const std::string &filename) const override;
};