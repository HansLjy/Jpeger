#include "CLI/CLI.hpp"
#include "ImageIO.hpp"
#include "Jpeger.hpp"

int main() {
    // CLI::App app("test");

    std::string input_filename(IMAGE_PATH "/test.bmp");
    std::string output_filename(IMAGE_PATH "/test.compress.bmp");
    std::string output_decompressed_filename(IMAGE_PATH "/test.decompressed.bmp");
    std::string config_filename(CONFIG_PATH "/test.json");

    std::ifstream config_file(config_filename);
    Json::Value config;
    config_file >> config;

    // app.add_option("-c,--config", config_file, "config file path");
    // app.add_option("-o,--output", output_file, "output file path");
    // app.add_option("-i,--input", input_file, "input file path");

    // CLI11_PARSE(app);

    auto io = new BMPIO;
    auto jpeger = new Jpeger(config);
    auto rgb = io->Read(input_filename);
    jpeger->Compress(rgb, output_filename);
    rgb = jpeger->Decompress(output_filename);
    io->Write(rgb, output_decompressed_filename);

    return 0;
}