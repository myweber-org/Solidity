#include <iostream>
#include <vector>
#include <cmath>
#include <cstdint>

struct Pixel {
    uint8_t r, g, b;
};

class Image {
private:
    std::vector<std::vector<Pixel>> data;
    int width, height;

public:
    Image(int w, int h) : width(w), height(h) {
        data.resize(height, std::vector<Pixel>(width, {0, 0, 0}));
    }

    void setPixel(int x, int y, Pixel p) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            data[y][x] = p;
        }
    }

    Pixel getPixel(int x, int y) const {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            return data[y][x];
        }
        return {0, 0, 0};
    }

    Image resize(int newWidth, int newHeight) const {
        Image resized(newWidth, newHeight);
        double xRatio = static_cast<double>(width) / newWidth;
        double yRatio = static_cast<double>(height) / newHeight;

        for (int y = 0; y < newHeight; ++y) {
            for (int x = 0; x < newWidth; ++x) {
                double srcX = x * xRatio;
                double srcY = y * yRatio;
                int xInt = static_cast<int>(srcX);
                int yInt = static_cast<int>(srcY);
                double xFrac = srcX - xInt;
                double yFrac = srcY - yInt;

                Pixel p00 = getPixel(xInt, yInt);
                Pixel p10 = getPixel(xInt + 1, yInt);
                Pixel p01 = getPixel(xInt, yInt + 1);
                Pixel p11 = getPixel(xInt + 1, yInt + 1);

                Pixel interpolated;
                interpolated.r = static_cast<uint8_t>(
                    (1 - xFrac) * (1 - yFrac) * p00.r +
                    xFrac * (1 - yFrac) * p10.r +
                    (1 - xFrac) * yFrac * p01.r +
                    xFrac * yFrac * p11.r
                );
                interpolated.g = static_cast<uint8_t>(
                    (1 - xFrac) * (1 - yFrac) * p00.g +
                    xFrac * (1 - yFrac) * p10.g +
                    (1 - xFrac) * yFrac * p01.g +
                    xFrac * yFrac * p11.g
                );
                interpolated.b = static_cast<uint8_t>(
                    (1 - xFrac) * (1 - yFrac) * p00.b +
                    xFrac * (1 - yFrac) * p10.b +
                    (1 - xFrac) * yFrac * p01.b +
                    xFrac * yFrac * p11.b
                );

                resized.setPixel(x, y, interpolated);
            }
        }
        return resized;
    }

    void printDimensions() const {
        std::cout << "Image dimensions: " << width << "x" << height << std::endl;
    }
};

int main() {
    Image original(4, 4);
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            original.setPixel(x, y, {static_cast<uint8_t>(x * 50), static_cast<uint8_t>(y * 50), 100});
        }
    }

    original.printDimensions();
    Image resized = original.resize(8, 8);
    resized.printDimensions();

    return 0;
}