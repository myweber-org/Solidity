
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
                data[y][x] = Pixel{
                    static_cast<unsigned char>(rand() % 256),
                    static_cast<unsigned char>(rand() % 256),
                    static_cast<unsigned char>(rand() % 256)
                };
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

            float dx = gx - gxi;
            float dy = gy - gyi;

            Pixel p1 = src.getPixel(gxi, gyi);
            Pixel p2 = src.getPixel(gxi + 1, gyi);
            Pixel p3 = src.getPixel(gxi, gyi + 1);
            Pixel p4 = src.getPixel(gxi + 1, gyi + 1);

            unsigned char r = static_cast<unsigned char>(
                p1.r * (1 - dx) * (1 - dy) +
                p2.r * dx * (1 - dy) +
                p3.r * (1 - dx) * dy +
                p4.r * dx * dy
            );
            unsigned char g = static_cast<unsigned char>(
                p1.g * (1 - dx) * (1 - dy) +
                p2.g * dx * (1 - dy) +
                p3.g * (1 - dx) * dy +
                p4.g * dx * dy
            );
            unsigned char b = static_cast<unsigned char>(
                p1.b * (1 - dx) * (1 - dy) +
                p2.b * dx * (1 - dy) +
                p3.b * (1 - dx) * dy +
                p4.b * dx * dy
            );

            dst.setPixel(x, y, Pixel{r, g, b});
        }
    }
    return dst;
}

void printImageInfo(const Image& img, const std::string& name) {
    std::cout << name << " - Width: " << img.getWidth()
              << ", Height: " << img.getHeight() << std::endl;
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    Image original(8, 6);
    original.fillRandom();
    printImageInfo(original, "Original");

    try {
        Image resized = resizeImage(original, 12, 9);
        printImageInfo(resized, "Resized");
        std::cout << "Image resizing completed successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}