
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

    Pixel getPixel(int x, int y) const {
        if (x < 0) x = 0;
        if (x >= width) x = width - 1;
        if (y < 0) y = 0;
        if (y >= height) y = height - 1;
        return data[y][x];
    }

    void setPixel(int x, int y, const Pixel& p) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            data[y][x] = p;
        }
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    static Image resize(const Image& src, int newWidth, int newHeight) {
        if (newWidth <= 0 || newHeight <= 0) {
            throw std::invalid_argument("Invalid dimensions");
        }

        Image dst(newWidth, newHeight);
        float xRatio = static_cast<float>(src.getWidth() - 1) / newWidth;
        float yRatio = static_cast<float>(src.getHeight() - 1) / newHeight;

        for (int y = 0; y < newHeight; ++y) {
            for (int x = 0; x < newWidth; ++x) {
                float gx = x * xRatio;
                float gy = y * yRatio;
                int gxi = static_cast<int>(gx);
                int gyi = static_cast<int>(gy);

                float dx = gx - gxi;
                float dy = gy - gyi;

                Pixel p00 = src.getPixel(gxi, gyi);
                Pixel p10 = src.getPixel(gxi + 1, gyi);
                Pixel p01 = src.getPixel(gxi, gyi + 1);
                Pixel p11 = src.getPixel(gxi + 1, gyi + 1);

                Pixel result;
                result.r = static_cast<unsigned char>(
                    (1 - dx) * (1 - dy) * p00.r + dx * (1 - dy) * p10.r +
                    (1 - dx) * dy * p01.r + dx * dy * p11.r
                );
                result.g = static_cast<unsigned char>(
                    (1 - dx) * (1 - dy) * p00.g + dx * (1 - dy) * p10.g +
                    (1 - dx) * dy * p01.g + dx * dy * p11.g
                );
                result.b = static_cast<unsigned char>(
                    (1 - dx) * (1 - dy) * p00.b + dx * (1 - dy) * p10.b +
                    (1 - dx) * dy * p01.b + dx * dy * p11.b
                );

                dst.setPixel(x, y, result);
            }
        }
        return dst;
    }

    void fillTestPattern() {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                Pixel p;
                p.r = static_cast<unsigned char>((x * 255) / width);
                p.g = static_cast<unsigned char>((y * 255) / height);
                p.b = static_cast<unsigned char>(((x + y) * 255) / (width + height));
                setPixel(x, y, p);
            }
        }
    }

    void printInfo() const {
        std::cout << "Image size: " << width << "x" << height << std::endl;
    }
};

int main() {
    try {
        Image original(8, 6);
        original.fillTestPattern();
        original.printInfo();

        Image resized = Image::resize(original, 16, 12);
        resized.printInfo();

        std::cout << "Image resizing completed successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}