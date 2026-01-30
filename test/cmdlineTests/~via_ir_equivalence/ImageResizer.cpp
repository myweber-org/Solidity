
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

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    Image resize(int newWidth, int newHeight) const {
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
                Pixel p;
                if ((x / 20 + y / 20) % 2 == 0) {
                    p = {255, 100, 50};
                } else {
                    p = {50, 150, 255};
                }
                setPixel(x, y, p);
            }
        }
    }

    void printInfo() const {
        std::cout << "Image size: " << width << "x" << height << std::endl;
        std::cout << "Top-left pixel: ("
                  << static_cast<int>(data[0][0].r) << ","
                  << static_cast<int>(data[0][0].g) << ","
                  << static_cast<int>(data[0][0].b) << ")" << std::endl;
    }
};

int main() {
    try {
        Image original(80, 60);
        original.fillCheckerPattern();
        std::cout << "Original image:" << std::endl;
        original.printInfo();

        Image resized = original.resize(120, 90);
        std::cout << "\nResized image:" << std::endl;
        resized.printInfo();

        Image small = original.resize(40, 30);
        std::cout << "\nSmall image:" << std::endl;
        small.printInfo();

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}