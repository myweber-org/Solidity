
#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

bool resizeImage(const fs::path& inputPath, const fs::path& outputPath, int newWidth, int newHeight) {
    cv::Mat image = cv::imread(inputPath.string());
    if (image.empty()) {
        std::cerr << "Error: Could not load image from " << inputPath << std::endl;
        return false;
    }

    cv::Mat resizedImage;
    cv::resize(image, resizedImage, cv::Size(newWidth, newHeight), 0, 0, cv::INTER_LINEAR);

    if (!cv::imwrite(outputPath.string(), resizedImage)) {
        std::cerr << "Error: Could not save image to " << outputPath << std::endl;
        return false;
    }

    std::cout << "Image resized successfully. Output saved to: " << outputPath << std::endl;
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <input_image> <output_image> <width> <height>" << std::endl;
        return 1;
    }

    fs::path inputPath(argv[1]);
    fs::path outputPath(argv[2]);
    int width = std::stoi(argv[3]);
    int height = std::stoi(argv[4]);

    if (!fs::exists(inputPath)) {
        std::cerr << "Error: Input file does not exist." << std::endl;
        return 1;
    }

    if (width <= 0 || height <= 0) {
        std::cerr << "Error: Width and height must be positive integers." << std::endl;
        return 1;
    }

    bool success = resizeImage(inputPath, outputPath, width, height);
    return success ? 0 : 1;
}