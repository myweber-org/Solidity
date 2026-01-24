
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "stb_image.h"
#include "stb_image_write.h"

namespace fs = std::filesystem;

struct ImageData {
    int width;
    int height;
    int channels;
    unsigned char* data;
};

ImageData loadImage(const std::string& path) {
    ImageData img{};
    img.data = stbi_load(path.c_str(), &img.width, &img.height, &img.channels, 0);
    if (!img.data) {
        throw std::runtime_error("Failed to load image: " + path);
    }
    return img;
}

void saveImage(const std::string& path, const ImageData& img) {
    std::string ext = fs::path(path).extension().string();
    int success = 0;

    if (ext == ".png") {
        success = stbi_write_png(path.c_str(), img.width, img.height, img.channels, img.data, img.width * img.channels);
    } else if (ext == ".jpg" || ext == ".jpeg") {
        success = stbi_write_jpg(path.c_str(), img.width, img.height, img.channels, img.data, 90);
    } else if (ext == ".bmp") {
        success = stbi_write_bmp(path.c_str(), img.width, img.height, img.channels, img.data);
    } else {
        throw std::runtime_error("Unsupported output format: " + ext);
    }

    if (!success) {
        throw std::runtime_error("Failed to write image: " + path);
    }
}

ImageData resizeImage(const ImageData& src, int newWidth, int newHeight) {
    if (newWidth <= 0 || newHeight <= 0) {
        throw std::invalid_argument("Invalid target dimensions");
    }

    ImageData dst{};
    dst.width = newWidth;
    dst.height = newHeight;
    dst.channels = src.channels;
    dst.data = new unsigned char[newWidth * newHeight * src.channels];

    float xRatio = static_cast<float>(src.width - 1) / newWidth;
    float yRatio = static_cast<float>(src.height - 1) / newHeight;

    for (int y = 0; y < newHeight; ++y) {
        for (int x = 0; x < newWidth; ++x) {
            int srcX = static_cast<int>(x * xRatio);
            int srcY = static_cast<int>(y * yRatio);

            for (int c = 0; c < src.channels; ++c) {
                dst.data[(y * newWidth + x) * src.channels + c] =
                    src.data[(srcY * src.width + srcX) * src.channels + c];
            }
        }
    }

    return dst;
}

void processImage(const std::string& inputPath, const std::string& outputPath, int targetWidth, int targetHeight) {
    ImageData original = loadImage(inputPath);
    
    std::cout << "Loaded image: " << inputPath << " (" 
              << original.width << "x" << original.height 
              << ", channels: " << original.channels << ")\n";

    ImageData resized = resizeImage(original, targetWidth, targetHeight);
    
    std::cout << "Resized to: " << targetWidth << "x" << targetHeight << "\n";
    
    saveImage(outputPath, resized);
    
    std::cout << "Saved to: " << outputPath << "\n";
    
    stbi_image_free(original.data);
    delete[] resized.data;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <input> <output> <width> <height>\n";
        return 1;
    }

    try {
        std::string inputPath = argv[1];
        std::string outputPath = argv[2];
        int width = std::stoi(argv[3]);
        int height = std::stoi(argv[4]);

        if (!fs::exists(inputPath)) {
            std::cerr << "Input file does not exist: " << inputPath << "\n";
            return 1;
        }

        processImage(inputPath, outputPath, width, height);
        std::cout << "Image processing completed successfully.\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}