
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

            Pixel c00 = src.getPixel(gxi, gyi);
            Pixel c10 = src.getPixel(gxi + 1, gyi);
            Pixel c01 = src.getPixel(gxi, gyi + 1);
            Pixel c11 = src.getPixel(gxi + 1, gyi + 1);

            Pixel result;
            result.r = static_cast<unsigned char>(
                (1 - tx) * (1 - ty) * c00.r +
                tx * (1 - ty) * c10.r +
                (1 - tx) * ty * c01.r +
                tx * ty * c11.r
            );
            result.g = static_cast<unsigned char>(
                (1 - tx) * (1 - ty) * c00.g +
                tx * (1 - ty) * c10.g +
                (1 - tx) * ty * c01.g +
                tx * ty * c11.g
            );
            result.b = static_cast<unsigned char>(
                (1 - tx) * (1 - ty) * c00.b +
                tx * (1 - ty) * c10.b +
                (1 - tx) * ty * c01.b +
                tx * ty * c11.b
            );

            dst.setPixel(x, y, result);
        }
    }
    return dst;
}

void printImageInfo(const Image& img) {
    std::cout << "Image size: " << img.getWidth() << "x" << img.getHeight() << std::endl;
}

int main() {
    try {
        Image original(800, 600);
        original.fillWithColor(100, 150, 200);

        std::cout << "Original ";
        printImageInfo(original);

        Image resized = resizeImage(original, 400, 300);

        std::cout << "Resized ";
        printImageInfo(resized);

        std::cout << "Image resizing completed successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}