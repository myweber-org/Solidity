
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

cv::Mat resizeImage(const cv::Mat& inputImage, int newWidth, int newHeight) {
    if (inputImage.empty()) {
        std::cerr << "Error: Input image is empty." << std::endl;
        return cv::Mat();
    }
    
    cv::Mat resizedImage;
    cv::resize(inputImage, resizedImage, cv::Size(newWidth, newHeight), 0, 0, cv::INTER_LINEAR);
    
    return resizedImage;
}

bool saveImage(const cv::Mat& image, const std::string& outputPath) {
    if (image.empty()) {
        std::cerr << "Error: Cannot save empty image." << std::endl;
        return false;
    }
    
    return cv::imwrite(outputPath, image);
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "Usage: " << argv[0] << " <input_image> <output_image> <width> <height>" << std::endl;
        return 1;
    }
    
    std::string inputPath = argv[1];
    std::string outputPath = argv[2];
    int newWidth = std::stoi(argv[3]);
    int newHeight = std::stoi(argv[4]);
    
    cv::Mat originalImage = cv::imread(inputPath);
    if (originalImage.empty()) {
        std::cerr << "Error: Could not load image from " << inputPath << std::endl;
        return 1;
    }
    
    cv::Mat resizedImage = resizeImage(originalImage, newWidth, newHeight);
    
    if (saveImage(resizedImage, outputPath)) {
        std::cout << "Image resized successfully and saved to " << outputPath << std::endl;
        std::cout << "Original dimensions: " << originalImage.cols << "x" << originalImage.rows << std::endl;
        std::cout << "New dimensions: " << resizedImage.cols << "x" << resizedImage.rows << std::endl;
        return 0;
    } else {
        std::cerr << "Error: Failed to save image to " << outputPath << std::endl;
        return 1;
    }
}