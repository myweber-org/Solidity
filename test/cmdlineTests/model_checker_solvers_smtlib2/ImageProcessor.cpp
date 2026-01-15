
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdint>

class ImageProcessor {
private:
    std::vector<std::vector<uint8_t>> pixelData;
    int width;
    int height;

public:
    ImageProcessor(int w, int h) : width(w), height(h) {
        pixelData.resize(height, std::vector<uint8_t>(width, 0));
    }

    void setPixel(int x, int y, uint8_t value) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            pixelData[y][x] = value;
        }
    }

    uint8_t getPixel(int x, int y) const {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            return pixelData[y][x];
        }
        return 0;
    }

    void convertToGrayscale() {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                uint8_t pixel = pixelData[y][x];
                uint8_t grayValue = static_cast<uint8_t>(0.299 * pixel + 0.587 * pixel + 0.114 * pixel);
                pixelData[y][x] = grayValue;
            }
        }
    }

    std::vector<std::vector<uint8_t>> detectEdges() const {
        std::vector<std::vector<uint8_t>> edgeMap(height, std::vector<uint8_t>(width, 0));
        const int sobelX[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
        const int sobelY[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

        for (int y = 1; y < height - 1; ++y) {
            for (int x = 1; x < width - 1; ++x) {
                int gradientX = 0;
                int gradientY = 0;

                for (int ky = -1; ky <= 1; ++ky) {
                    for (int kx = -1; kx <= 1; ++kx) {
                        uint8_t pixel = getPixel(x + kx, y + ky);
                        gradientX += sobelX[ky + 1][kx + 1] * pixel;
                        gradientY += sobelY[ky + 1][kx + 1] * pixel;
                    }
                }

                int magnitude = static_cast<int>(std::sqrt(gradientX * gradientX + gradientY * gradientY));
                edgeMap[y][x] = static_cast<uint8_t>(std::min(255, magnitude));
            }
        }
        return edgeMap;
    }

    void printImage() const {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                std::cout << static_cast<int>(pixelData[y][x]) << " ";
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    ImageProcessor img(5, 5);
    for (int y = 0; y < 5; ++y) {
        for (int x = 0; x < 5; ++x) {
            img.setPixel(x, y, static_cast<uint8_t>((x + y) * 25));
        }
    }

    std::cout << "Original Image:" << std::endl;
    img.printImage();

    img.convertToGrayscale();
    std::cout << "\nGrayscale Image:" << std::endl;
    img.printImage();

    auto edges = img.detectEdges();
    std::cout << "\nEdge Detection Result:" << std::endl;
    for (const auto& row : edges) {
        for (uint8_t val : row) {
            std::cout << static_cast<int>(val) << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}