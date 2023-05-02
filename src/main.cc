#include "ImageIO.hpp"
#include "Jpeger.hpp"
#include <fstream>

int main() {
    // CLI::App app("test");

    std::string input_filename(IMAGE_PATH "/test.bmp");
    std::string output_filename(IMAGE_PATH "/test.bmp.compressed");
    std::string output_decompressed_filename(IMAGE_PATH "/test.decompressed.bmp");
    std::string config_filename(CONFIG_PATH "/test.json");

    std::ifstream config_file(config_filename);
    Json::Value config;
    config_file >> config;

    auto io = new BMPIO;
    auto jpeger = new Jpeger(config);
    auto rgb = io->Read(input_filename);
    jpeger->Compress(rgb, output_filename);
    rgb = jpeger->Decompress(output_filename);
    io->Write(rgb, output_decompressed_filename);

    return 0;
}