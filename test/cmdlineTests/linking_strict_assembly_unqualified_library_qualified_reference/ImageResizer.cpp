
#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

bool resizeImage(const std::string& inputPath, const std::string& outputPath, int targetWidth, int targetHeight, bool keepAspectRatio) {
    cv::Mat image = cv::imread(inputPath);
    if (image.empty()) {
        std::cerr << "Error: Could not load image from " << inputPath << std::endl;
        return false;
    }

    cv::Size targetSize(targetWidth, targetHeight);
    cv::Mat resizedImage;

    if (keepAspectRatio) {
        double aspectRatio = static_cast<double>(image.cols) / image.rows;
        if (targetWidth / aspectRatio <= targetHeight) {
            targetSize.height = static_cast<int>(targetWidth / aspectRatio);
        } else {
            targetSize.width = static_cast<int>(targetHeight * aspectRatio);
        }
    }

    cv::resize(image, resizedImage, targetSize, 0, 0, cv::INTER_LANCZOS4);

    if (!cv::imwrite(outputPath, resizedImage)) {
        std::cerr << "Error: Could not save image to " << outputPath << std::endl;
        return false;
    }

    std::cout << "Image resized successfully: " << outputPath << std::endl;
    return true;
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cout << "Usage: " << argv[0] << " <input_path> <output_path> <width> <height> [keep_aspect]" << std::endl;
        std::cout << "Example: " << argv[0] << " input.jpg output.jpg 800 600 1" << std::endl;
        return 1;
    }

    std::string inputPath = argv[1];
    std::string outputPath = argv[2];
    int width = std::stoi(argv[3]);
    int height = std::stoi(argv[4]);
    bool keepAspect = (argc > 5) ? std::stoi(argv[5]) != 0 : true;

    if (!fs::exists(inputPath)) {
        std::cerr << "Error: Input file does not exist." << std::endl;
        return 1;
    }

    if (width <= 0 || height <= 0) {
        std::cerr << "Error: Width and height must be positive integers." << std::endl;
        return 1;
    }

    return resizeImage(inputPath, outputPath, width, height, keepAspect) ? 0 : 1;
}