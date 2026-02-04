
#include <iostream>
#include <opencv2/opencv.hpp>
#include <filesystem>

namespace fs = std::filesystem;

bool resizeImage(const fs::path& inputPath, const fs::path& outputDir, int width, int height) {
    cv::Mat image = cv::imread(inputPath.string());
    if (image.empty()) {
        std::cerr << "Error: Could not load image " << inputPath << std::endl;
        return false;
    }

    cv::Mat resizedImage;
    cv::resize(image, resizedImage, cv::Size(width, height));

    fs::path outputPath = outputDir / inputPath.filename();
    if (!cv::imwrite(outputPath.string(), resizedImage)) {
        std::cerr << "Error: Could not save image to " << outputPath << std::endl;
        return false;
    }

    std::cout << "Resized " << inputPath.filename() << " and saved to " << outputPath << std::endl;
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <input_path> <output_dir> <width> <height>" << std::endl;
        return 1;
    }

    fs::path inputPath(argv[1]);
    fs::path outputDir(argv[2]);
    int width = std::stoi(argv[3]);
    int height = std::stoi(argv[4]);

    if (!fs::exists(inputPath)) {
        std::cerr << "Error: Input path does not exist." << std::endl;
        return 1;
    }

    if (!fs::exists(outputDir)) {
        if (!fs::create_directories(outputDir)) {
            std::cerr << "Error: Could not create output directory." << std::endl;
            return 1;
        }
    }

    if (fs::is_directory(inputPath)) {
        for (const auto& entry : fs::directory_iterator(inputPath)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".bmp") {
                    resizeImage(entry.path(), outputDir, width, height);
                }
            }
        }
    } else if (fs::is_regular_file(inputPath)) {
        std::string ext = inputPath.extension().string();
        if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".bmp") {
            if (!resizeImage(inputPath, outputDir, width, height)) {
                return 1;
            }
        } else {
            std::cerr << "Error: Unsupported file format." << std::endl;
            return 1;
        }
    } else {
        std::cerr << "Error: Invalid input path." << std::endl;
        return 1;
    }

    return 0;
}