
#include <iostream>
#include <vector>
#include <cmath>
#include <stdexcept>

class Image {
private:
    std::vector<std::vector<int>> pixels;
    int width, height;

public:
    Image(int w, int h) : width(w), height(h) {
        pixels.resize(height, std::vector<int>(width, 0));
    }

    void setPixel(int x, int y, int value) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            pixels[y][x] = value;
        }
    }

    int getPixel(int x, int y) const {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            return pixels[y][x];
        }
        return 0;
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    Image resize(int newWidth, int newHeight) const {
        if (newWidth <= 0 || newHeight <= 0) {
            throw std::invalid_argument("New dimensions must be positive");
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

                float dx = gx - gxi;
                float dy = gy - gyi;

                int p1 = getPixel(gxi, gyi);
                int p2 = getPixel(gxi + 1, gyi);
                int p3 = getPixel(gxi, gyi + 1);
                int p4 = getPixel(gxi + 1, gyi + 1);

                float val = p1 * (1 - dx) * (1 - dy) +
                            p2 * dx * (1 - dy) +
                            p3 * (1 - dx) * dy +
                            p4 * dx * dy;

                resized.setPixel(x, y, static_cast<int>(std::round(val)));
            }
        }
        return resized;
    }

    void print() const {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                std::cout << pixels[y][x] << " ";
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    Image original(4, 4);
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            original.setPixel(x, y, (x + y) * 10);
        }
    }

    std::cout << "Original image:" << std::endl;
    original.print();

    try {
        Image resized = original.resize(8, 8);
        std::cout << "\nResized image:" << std::endl;
        resized.print();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}#include <iostream>
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
                interpolated.r = static_cast<uint8_t>(
                    (1 - tx) * (1 - ty) * p00.r +
                    tx * (1 - ty) * p10.r +
                    (1 - tx) * ty * p01.r +
                    tx * ty * p11.r
                );
                interpolated.g = static_cast<uint8_t>(
                    (1 - tx) * (1 - ty) * p00.g +
                    tx * (1 - ty) * p10.g +
                    (1 - tx) * ty * p01.g +
                    tx * ty * p11.g
                );
                interpolated.b = static_cast<uint8_t>(
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

    void printDimensions() const {
        std::cout << "Image dimensions: " << width << "x" << height << std::endl;
    }
};

int main() {
    Image original(4, 4);
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            Pixel p = {
                static_cast<uint8_t>(x * 64),
                static_cast<uint8_t>(y * 64),
                static_cast<uint8_t>((x + y) * 32)
            };
            original.setPixel(x, y, p);
        }
    }

    original.printDimensions();
    Image resized = original.resize(8, 8);
    resized.printDimensions();

    return 0;
}