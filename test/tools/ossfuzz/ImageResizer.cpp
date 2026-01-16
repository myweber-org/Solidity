
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

    void fillCheckerboard() {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                Pixel p;
                if ((x / 10 + y / 10) % 2 == 0) {
                    p = {255, 255, 255};
                } else {
                    p = {0, 0, 0};
                }
                setPixel(x, y, p);
            }
        }
    }

    void printTopLeft(int size) const {
        for (int y = 0; y < std::min(size, height); ++y) {
            for (int x = 0; x < std::min(size, width); ++x) {
                Pixel p = getPixel(x, y);
                std::cout << "(" << static_cast<int>(p.r) << ","
                          << static_cast<int>(p.g) << ","
                          << static_cast<int>(p.b) << ") ";
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    Image original(50, 30);
    original.fillCheckerboard();
    
    std::cout << "Original image top-left 5x5:" << std::endl;
    original.printTopLeft(5);
    
    Image resized = original.resize(25, 15);
    
    std::cout << "\nResized image top-left 5x5:" << std::endl;
    resized.printTopLeft(5);
    
    return 0;
}