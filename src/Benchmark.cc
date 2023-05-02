#include "ImageIO.hpp"
#include "Jpeger.hpp"
#include <fstream>
#include <filesystem>

const std::string test_list[] = {
    "artificial.ppm",    "bridge.ppm",     "fireworks.ppm",      "leaves_iso_1600.ppm",    "nightshot_iso_1600.ppm",
    "big_building.ppm",  "cathedral.ppm",  "flower_foveon.ppm",  "leaves_iso_200.ppm",
    "big_tree.ppm",      "deer.ppm",       "hdr.ppm",            "nightshot_iso_100.ppm",  "spider_web.ppm"
};

int main() {
    std::string config_filename(CONFIG_PATH "/test.json");
    std::ifstream config_file(config_filename);
    Json::Value config;
    config_file >> config;
    auto jpeger = new Jpeger(config);
    auto input = new PPMIO;
    auto output = new BMPIO;

    for (const auto& filename : test_list) {
        std::string input_filename(IMAGE_PATH "/test/" + filename);
        std::string output_filename(IMAGE_PATH "/test/" + filename + ".compressed");
        std::string output_decompressed_filename(IMAGE_PATH "/test/" + filename + ".decompressed.bmp");

        auto rgb = input->Read(input_filename);
        jpeger->Compress(rgb, output_filename);
        rgb = jpeger->Decompress(output_filename);
        output->Write(rgb, output_decompressed_filename);
    }

    return 0;
}