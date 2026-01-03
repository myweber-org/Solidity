#include <iostream>
#include <vector>
#include <cmath>
#include <opencv2/opencv.hpp>

class ImageProcessor {
public:
    static cv::Mat convertToGrayscale(const cv::Mat& inputImage) {
        if (inputImage.empty()) {
            throw std::invalid_argument("Input image is empty");
        }
        cv::Mat grayscaleImage;
        if (inputImage.channels() == 3) {
            cv::cvtColor(inputImage, grayscaleImage, cv::COLOR_BGR2GRAY);
        } else if (inputImage.channels() == 4) {
            cv::cvtColor(inputImage, grayscaleImage, cv::COLOR_BGRA2GRAY);
        } else {
            grayscaleImage = inputImage.clone();
        }
        return grayscaleImage;
    }

    static cv::Mat detectEdges(const cv::Mat& inputImage, double lowThreshold = 50.0, double highThreshold = 150.0) {
        if (inputImage.empty()) {
            throw std::invalid_argument("Input image is empty");
        }
        cv::Mat grayscaleImage = convertToGrayscale(inputImage);
        cv::Mat blurredImage;
        cv::GaussianBlur(grayscaleImage, blurredImage, cv::Size(5, 5), 1.5);
        cv::Mat edgeImage;
        cv::Canny(blurredImage, edgeImage, lowThreshold, highThreshold);
        return edgeImage;
    }

    static void saveImage(const cv::Mat& image, const std::string& filePath) {
        if (image.empty()) {
            throw std::invalid_argument("Image is empty, cannot save");
        }
        if (!cv::imwrite(filePath, image)) {
            throw std::runtime_error("Failed to save image to: " + filePath);
        }
    }

    static void displayImage(const cv::Mat& image, const std::string& windowName = "Image") {
        if (image.empty()) {
            throw std::invalid_argument("Image is empty, cannot display");
        }
        cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);
        cv::imshow(windowName, image);
        cv::waitKey(0);
        cv::destroyWindow(windowName);
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <image_path>" << std::endl;
        return 1;
    }

    try {
        std::string imagePath = argv[1];
        cv::Mat originalImage = cv::imread(imagePath, cv::IMREAD_COLOR);
        if (originalImage.empty()) {
            std::cerr << "Error: Could not load image from " << imagePath << std::endl;
            return 1;
        }

        cv::Mat grayscaleImage = ImageProcessor::convertToGrayscale(originalImage);
        cv::Mat edgeImage = ImageProcessor::detectEdges(originalImage);

        ImageProcessor::saveImage(grayscaleImage, "grayscale_output.jpg");
        ImageProcessor::saveImage(edgeImage, "edges_output.jpg");

        std::cout << "Processing completed. Output saved as 'grayscale_output.jpg' and 'edges_output.jpg'" << std::endl;

        ImageProcessor::displayImage(originalImage, "Original Image");
        ImageProcessor::displayImage(grayscaleImage, "Grayscale Image");
        ImageProcessor::displayImage(edgeImage, "Edge Detection");

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}