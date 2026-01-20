
#include <iostream>
#include <vector>
#include <cmath>

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

    void rotate90Clockwise() {
        std::vector<std::vector<Pixel>> rotated(width, std::vector<Pixel>(height));
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                rotated[x][height - 1 - y] = data[y][x];
            }
        }
        data = rotated;
        std::swap(width, height);
    }

    void convertToGrayscale() {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                unsigned char gray = static_cast<unsigned char>(
                    0.299 * data[y][x].r + 0.587 * data[y][x].g + 0.114 * data[y][x].b
                );
                data[y][x] = {gray, gray, gray};
            }
        }
    }

    void printStats() const {
        std::cout << "Image dimensions: " << width << "x" << height << std::endl;
    }
};

int main() {
    Image img(3, 2);
    img.setPixel(0, 0, {255, 0, 0});
    img.setPixel(1, 0, {0, 255, 0});
    img.setPixel(2, 0, {0, 0, 255});
    img.setPixel(0, 1, {128, 128, 128});
    img.setPixel(1, 1, {64, 192, 64});
    img.setPixel(2, 1, {192, 64, 192});

    img.printStats();
    img.rotate90Clockwise();
    img.printStats();
    img.convertToGrayscale();

    return 0;
}