
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

    void fillWithColor(unsigned char r, unsigned char g, unsigned char b) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                data[y][x] = {r, g, b};
            }
        }
    }
};

Image resizeImage(const Image& src, int newWidth, int newHeight) {
    if (newWidth <= 0 || newHeight <= 0) {
        throw std::invalid_argument("New dimensions must be positive");
    }

    Image dest(newWidth, newHeight);
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

            Pixel result;
            result.r = static_cast<unsigned char>(
                (1 - tx) * (1 - ty) * p00.r +
                tx * (1 - ty) * p10.r +
                (1 - tx) * ty * p01.r +
                tx * ty * p11.r
            );
            result.g = static_cast<unsigned char>(
                (1 - tx) * (1 - ty) * p00.g +
                tx * (1 - ty) * p10.g +
                (1 - tx) * ty * p01.g +
                tx * ty * p11.g
            );
            result.b = static_cast<unsigned char>(
                (1 - tx) * (1 - ty) * p00.b +
                tx * (1 - ty) * p10.b +
                (1 - tx) * ty * p01.b +
                tx * ty * p11.b
            );

            dest.setPixel(x, y, result);
        }
    }
    return dest;
}

void printImageInfo(const Image& img) {
    std::cout << "Image size: " << img.getWidth() << "x" << img.getHeight() << std::endl;
    Pixel p = img.getPixel(0, 0);
    std::cout << "Top-left pixel: R=" << static_cast<int>(p.r)
              << " G=" << static_cast<int>(p.g)
              << " B=" << static_cast<int>(p.b) << std::endl;
}

int main() {
    try {
        Image original(4, 4);
        original.fillWithColor(100, 150, 200);

        std::cout << "Original image:" << std::endl;
        printImageInfo(original);

        Image resized = resizeImage(original, 8, 8);

        std::cout << "\nResized image:" << std::endl;
        printImageInfo(resized);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}