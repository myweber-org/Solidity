
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

    void fillRandom() {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                data[y][x] = { static_cast<unsigned char>(rand() % 256),
                               static_cast<unsigned char>(rand() % 256),
                               static_cast<unsigned char>(rand() % 256) };
            }
        }
    }
};

Image resizeImage(const Image& src, int newWidth, int newHeight) {
    if (newWidth <= 0 || newHeight <= 0) {
        throw std::invalid_argument("New dimensions must be positive");
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

            float tx = gx - gxi;
            float ty = gy - gyi;

            Pixel p00 = src.getPixel(gxi, gyi);
            Pixel p10 = src.getPixel(gxi + 1, gyi);
            Pixel p01 = src.getPixel(gxi, gyi + 1);
            Pixel p11 = src.getPixel(gxi + 1, gyi + 1);

            auto interpolate = [](float t, unsigned char a, unsigned char b) {
                return static_cast<unsigned char>((1 - t) * a + t * b);
            };

            Pixel top, bottom, result;
            top.r = interpolate(tx, p00.r, p10.r);
            top.g = interpolate(tx, p00.g, p10.g);
            top.b = interpolate(tx, p00.b, p10.b);

            bottom.r = interpolate(tx, p01.r, p11.r);
            bottom.g = interpolate(tx, p01.g, p11.g);
            bottom.b = interpolate(tx, p01.b, p11.b);

            result.r = interpolate(ty, top.r, bottom.r);
            result.g = interpolate(ty, top.g, bottom.g);
            result.b = interpolate(ty, top.b, bottom.b);

            dst.setPixel(x, y, result);
        }
    }
    return dst;
}

void printImageInfo(const Image& img, const std::string& name) {
    std::cout << name << " dimensions: " << img.getWidth() 
              << "x" << img.getHeight() << std::endl;
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    Image original(800, 600);
    original.fillRandom();
    printImageInfo(original, "Original");

    try {
        Image resized = resizeImage(original, 400, 300);
        printImageInfo(resized, "Resized");

        Image enlarged = resizeImage(original, 1200, 900);
        printImageInfo(enlarged, "Enlarged");

        std::cout << "Image resizing completed successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}