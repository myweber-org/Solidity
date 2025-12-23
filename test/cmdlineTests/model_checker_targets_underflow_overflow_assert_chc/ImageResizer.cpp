
#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

class ImageResizer {
public:
    ImageResizer(const std::string& inputPath, const std::string& outputPath, int targetWidth)
        : inputPath_(inputPath), outputPath_(outputPath), targetWidth_(targetWidth) {}

    bool process() {
        cv::Mat image = cv::imread(inputPath_);
        if (image.empty()) {
            std::cerr << "Error: Could not load image from " << inputPath_ << std::endl;
            return false;
        }

        double aspectRatio = static_cast<double>(image.cols) / image.rows;
        int targetHeight = static_cast<int>(targetWidth_ / aspectRatio);

        cv::Mat resizedImage;
        cv::resize(image, resizedImage, cv::Size(targetWidth_, targetHeight), 0, 0, cv::INTER_LANCZOS4);

        if (!cv::imwrite(outputPath_, resizedImage)) {
            std::cerr << "Error: Could not save image to " << outputPath_ << std::endl;
            return false;
        }

        std::cout << "Image resized successfully. New dimensions: " 
                  << targetWidth_ << "x" << targetHeight << std::endl;
        return true;
    }

private:
    std::string inputPath_;
    std::string outputPath_;
    int targetWidth_;
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <input_image> <output_image> <target_width>" << std::endl;
        return 1;
    }

    std::string inputPath = argv[1];
    std::string outputPath = argv[2];
    int targetWidth = std::stoi(argv[3]);

    if (!fs::exists(inputPath)) {
        std::cerr << "Error: Input file does not exist." << std::endl;
        return 1;
    }

    if (targetWidth <= 0) {
        std::cerr << "Error: Target width must be positive." << std::endl;
        return 1;
    }

    ImageResizer resizer(inputPath, outputPath, targetWidth);
    return resizer.process() ? 0 : 1;
}