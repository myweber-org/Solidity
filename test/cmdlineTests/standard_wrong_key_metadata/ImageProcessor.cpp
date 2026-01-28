#include <iostream>
#include <vector>
#include <cmath>
#include <stdexcept>

struct Pixel {
    unsigned char r, g, b;
};

class Image {
private:
    std::vector<std::vector<Pixel>> data;
    int width, height;

public:
    Image(int w, int h) : width(w), height(h) {
        data.resize(height, std::vector<Pixel>(width));
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

    Pixel getBilinearPixel(float x, float y) const {
        int x1 = static_cast<int>(std::floor(x));
        int y1 = static_cast<int>(std::floor(y));
        int x2 = x1 + 1;
        int y2 = y1 + 1;

        float dx = x - x1;
        float dy = y - y1;

        Pixel p11 = getPixel(x1, y1);
        Pixel p12 = getPixel(x1, y2);
        Pixel p21 = getPixel(x2, y1);
        Pixel p22 = getPixel(x2, y2);

        auto interpolate = [](float t, unsigned char a, unsigned char b) {
            return static_cast<unsigned char>((1 - t) * a + t * b);
        };

        Pixel result;
        result.r = interpolate(dy, interpolate(dx, p11.r, p21.r), interpolate(dx, p12.r, p22.r));
        result.g = interpolate(dy, interpolate(dx, p11.g, p21.g), interpolate(dx, p12.g, p22.g));
        result.b = interpolate(dy, interpolate(dx, p11.b, p21.b), interpolate(dx, p12.b, p22.b));

        return result;
    }

    Image rotate(float angle) const {
        float rad = angle * M_PI / 180.0;
        float cosA = std::cos(rad);
        float sinA = std::sin(rad);

        float centerX = width / 2.0f;
        float centerY = height / 2.0f;

        int newWidth = static_cast<int>(std::abs(width * cosA) + std::abs(height * sinA));
        int newHeight = static_cast<int>(std::abs(width * sinA) + std::abs(height * cosA));

        Image rotated(newWidth, newHeight);

        for (int y = 0; y < newHeight; ++y) {
            for (int x = 0; x < newWidth; ++x) {
                float srcX = (x - newWidth / 2.0f) * cosA + (y - newHeight / 2.0f) * sinA + centerX;
                float srcY = -(x - newWidth / 2.0f) * sinA + (y - newHeight / 2.0f) * cosA + centerY;

                if (srcX >= 0 && srcX < width - 1 && srcY >= 0 && srcY < height - 1) {
                    rotated.setPixel(x, y, getBilinearPixel(srcX, srcY));
                }
            }
        }

        return rotated;
    }

    void printDimensions() const {
        std::cout << "Image dimensions: " << width << "x" << height << std::endl;
    }
};

int main() {
    Image img(100, 100);
    
    for (int y = 0; y < 100; ++y) {
        for (int x = 0; x < 100; ++x) {
            Pixel p;
            p.r = static_cast<unsigned char>((x + y) % 256);
            p.g = static_cast<unsigned char>((x * 2) % 256);
            p.b = static_cast<unsigned char>((y * 2) % 256);
            img.setPixel(x, y, p);
        }
    }

    std::cout << "Original image:" << std::endl;
    img.printDimensions();

    Image rotated = img.rotate(45.0f);
    
    std::cout << "Rotated image:" << std::endl;
    rotated.printDimensions();

    return 0;
}