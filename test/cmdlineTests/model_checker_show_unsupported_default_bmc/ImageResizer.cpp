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

    void fillTestPattern() {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                Pixel p;
                p.r = static_cast<unsigned char>((x * 255) / width);
                p.g = static_cast<unsigned char>((y * 255) / height);
                p.b = static_cast<unsigned char>(((x + y) * 255) / (width + height));
                at(x, y) = p;
            }
        }
    }

    void printDimensions() const {
        std::cout << "Image size: " << width << "x" << height << std::endl;
    }
};

int main() {
    try {
        Image original(8, 6);
        original.fillTestPattern();
        original.printDimensions();

        Image resized = original.resize(16, 12);
        resized.printDimensions();

        std::cout << "Image resizing completed successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}