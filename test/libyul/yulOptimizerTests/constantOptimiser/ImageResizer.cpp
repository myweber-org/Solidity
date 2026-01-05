
#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

bool resizeImage(const std::string& inputPath, const std::string& outputPath, int width, int height) {
    cv::Mat image = cv::imread(inputPath);
    
    if (image.empty()) {
        std::cerr << "Error: Could not load image from " << inputPath << std::endl;
        return false;
    }
    
    cv::Mat resizedImage;
    cv::resize(image, resizedImage, cv::Size(width, height), 0, 0, cv::INTER_LINEAR);
    
    if (!cv::imwrite(outputPath, resizedImage)) {
        std::cerr << "Error: Could not save image to " << outputPath << std::endl;
        return false;
    }
    
    std::cout << "Image resized successfully: " << outputPath << std::endl;
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <input_image> <output_image> <width> <height>" << std::endl;
        return 1;
    }
    
    std::string inputPath = argv[1];
    std::string outputPath = argv[2];
    int width = std::stoi(argv[3]);
    int height = std::stoi(argv[4]);
    
    if (!fs::exists(inputPath)) {
        std::cerr << "Error: Input file does not exist" << std::endl;
        return 1;
    }
    
    if (width <= 0 || height <= 0) {
        std::cerr << "Error: Width and height must be positive integers" << std::endl;
        return 1;
    }
    
    bool success = resizeImage(inputPath, outputPath, width, height);
    
    return success ? 0 : 1;
}