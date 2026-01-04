
#include <opencv2/opencv.hpp>
#include <string>

bool resizeImage(const std::string& inputPath, const std::string& outputPath, int newWidth, int newHeight) {
    cv::Mat image = cv::imread(inputPath);
    if (image.empty()) {
        return false;
    }
    
    cv::Mat resizedImage;
    cv::resize(image, resizedImage, cv::Size(newWidth, newHeight), 0, 0, cv::INTER_LINEAR);
    
    return cv::imwrite(outputPath, resizedImage);
}