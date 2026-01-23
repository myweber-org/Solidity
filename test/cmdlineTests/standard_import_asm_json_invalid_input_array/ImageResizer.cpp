#include <iostream>
#include <vector>
#include <cmath>
#include <stdexcept>

struct Pixel {
    unsigned char r, g, b;
};

class Image {
private:
    int width, height;
    std::vector<Pixel> data;

public:
    Image(int w, int h) : width(w), height(h), data(w * h) {}

    Pixel& at(int x, int y) {
        if (x < 0 || x >= width || y < 0 || y >= height) {
            throw std::out_of_range("Pixel coordinates out of bounds");
        }
        return data[y * width + x];
    }

    const Pixel& at(int x, int y) const {
        if (x < 0 || x >= width || y < 0 || y >= height) {
            throw std::out_of_range("Pixel coordinates out of bounds");
        }
        return data[y * width + x];
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    Image resize(int newWidth, int newHeight) const {
        Image result(newWidth, newHeight);
        float xRatio = static_cast<float>(width - 1) / newWidth;
        float yRatio = static_cast<float>(height - 1) / newHeight;

        for (int y = 0; y < newHeight; ++y) {
            for (int x = 0; x < newWidth; ++x) {
                float gx = x * xRatio;
                float gy = y * yRatio;
                int gxi = static_cast<int>(gx);
                int gyi = static_cast<int>(gy);

                float tx = gx - gxi;
                float ty = gy - gyi;

                Pixel p00 = at(gxi, gyi);
                Pixel p10 = at(std::min(gxi + 1, width - 1), gyi);
                Pixel p01 = at(gxi, std::min(gyi + 1, height - 1));
                Pixel p11 = at(std::min(gxi + 1, width - 1), std::min(gyi + 1, height - 1));

                Pixel interpolated;
                interpolated.r = static_cast<unsigned char>(
                    (1 - tx) * (1 - ty) * p00.r +
                    tx * (1 - ty) * p10.r +
                    (1 - tx) * ty * p01.r +
                    tx * ty * p11.r
                );
                interpolated.g = static_cast<unsigned char>(
                    (1 - tx) * (1 - ty) * p00.g +
                    tx * (1 - ty) * p10.g +
                    (1 - tx) * ty * p01.g +
                    tx * ty * p11.g
                );
                interpolated.b = static_cast<unsigned char>(
                    (1 - tx) * (1 - ty) * p00.b +
                    tx * (1 - ty) * p10.b +
                    (1 - tx) * ty * p01.b +
                    tx * ty * p11.b
                );

                result.at(x, y) = interpolated;
            }
        }
        return result;
    }

    void fillCheckerPattern() {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                Pixel& p = at(x, y);
                if ((x / 20 + y / 20) % 2 == 0) {
                    p.r = 255; p.g = 200; p.b = 100;
                } else {
                    p.r = 50; p.g = 150; p.b = 255;
                }
            }
        }
    }

    void printTopLeft(int limit = 5) const {
        for (int y = 0; y < std::min(limit, height); ++y) {
            for (int x = 0; x < std::min(limit, width); ++x) {
                const Pixel& p = at(x, y);
                std::cout << "(" << static_cast<int>(p.r) << ","
                          << static_cast<int>(p.g) << ","
                          << static_cast<int>(p.b) << ") ";
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    Image original(100, 80);
    original.fillCheckerPattern();
    std::cout << "Original image top-left pixels:" << std::endl;
    original.printTopLeft();

    Image resized = original.resize(60, 40);
    std::cout << "\nResized image top-left pixels:" << std::endl;
    resized.printTopLeft();

    return 0;
}