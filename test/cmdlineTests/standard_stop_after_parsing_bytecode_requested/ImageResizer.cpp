
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

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    Image resize(int newWidth, int newHeight) const {
        if (newWidth <= 0 || newHeight <= 0) {
            throw std::invalid_argument("Invalid dimensions");
        }

        Image resized(newWidth, newHeight);
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

                Pixel p00 = getPixel(gxi, gyi);
                Pixel p10 = getPixel(gxi + 1, gyi);
                Pixel p01 = getPixel(gxi, gyi + 1);
                Pixel p11 = getPixel(gxi + 1, gyi + 1);

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

                resized.setPixel(x, y, interpolated);
            }
        }
        return resized;
    }

    void fillCheckerPattern() {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                bool isBlack = ((x / 20) + (y / 20)) % 2 == 0;
                setPixel(x, y, isBlack ? Pixel{0, 0, 0} : Pixel{255, 255, 255});
            }
        }
    }

    void printTopLeft(int limit = 5) const {
        for (int y = 0; y < std::min(limit, height); ++y) {
            for (int x = 0; x < std::min(limit, width); ++x) {
                Pixel p = getPixel(x, y);
                std::cout << "(" << (int)p.r << "," << (int)p.g << "," << (int)p.b << ") ";
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    try {
        Image original(100, 80);
        original.fillCheckerPattern();
        std::cout << "Original image top-left pixels:" << std::endl;
        original.printTopLeft();

        Image resized = original.resize(60, 40);
        std::cout << "\nResized image top-left pixels:" << std::endl;
        resized.printTopLeft();

        std::cout << "\nOriginal dimensions: " << original.getWidth() << "x" << original.getHeight() << std::endl;
        std::cout << "Resized dimensions: " << resized.getWidth() << "x" << resized.getHeight() << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}