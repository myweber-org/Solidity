
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

bool resizeImage(const std::string& inputPath, const std::string& outputPath, int width, int height) {
    cv::Mat image = cv::imread(inputPath);
    if (image.empty()) {
        std::cerr << "Error: Could not load image from " << inputPath << std::endl;
        return false;
    }

    cv::Mat resizedImage;
    cv::resize(image, resizedImage, cv::Size(width, height));

    if (!cv::imwrite(outputPath, resizedImage)) {
        std::cerr << "Error: Could not save image to " << outputPath << std::endl;
        return false;
    }

    std::cout << "Image resized successfully. Output saved to " << outputPath << std::endl;
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <input_path> <output_path> <width> <height>" << std::endl;
        return 1;
    }

    std::string inputPath = argv[1];
    std::string outputPath = argv[2];
    int width = std::stoi(argv[3]);
    int height = std::stoi(argv[4]);

    if (width <= 0 || height <= 0) {
        std::cerr << "Error: Width and height must be positive integers." << std::endl;
        return 1;
    }

    if (!resizeImage(inputPath, outputPath, width, height)) {
        return 1;
    }

    return 0;
}