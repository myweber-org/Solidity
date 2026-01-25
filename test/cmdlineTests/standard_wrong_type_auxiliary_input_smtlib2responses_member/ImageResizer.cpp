
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
            throw std::out_of_range("Pixel index out of bounds");
        }
        return data[y * width + x];
    }

    const Pixel& at(int x, int y) const {
        if (x < 0 || x >= width || y < 0 || y >= height) {
            throw std::out_of_range("Pixel index out of bounds");
        }
        return data[y * width + x];
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    Image resize(int newWidth, int newHeight) const {
        Image result(newWidth, newHeight);
        double xRatio = static_cast<double>(width - 1) / newWidth;
        double yRatio = static_cast<double>(height - 1) / newHeight;

        for (int y = 0; y < newHeight; ++y) {
            for (int x = 0; x < newWidth; ++x) {
                double gx = x * xRatio;
                double gy = y * yRatio;
                int gxi = static_cast<int>(gx);
                int gyi = static_cast<int>(gy);

                double tx = gx - gxi;
                double ty = gy - gyi;

                const Pixel& p00 = at(gxi, gyi);
                const Pixel& p10 = at(std::min(gxi + 1, width - 1), gyi);
                const Pixel& p01 = at(gxi, std::min(gyi + 1, height - 1));
                const Pixel& p11 = at(std::min(gxi + 1, width - 1), std::min(gyi + 1, height - 1));

                Pixel interpolated;
                interpolated.r = static_cast<unsigned char>(
                    (1 - tx) * (1 - ty) * p00.r + tx * (1 - ty) * p10.r +
                    (1 - tx) * ty * p01.r + tx * ty * p11.r
                );
                interpolated.g = static_cast<unsigned char>(
                    (1 - tx) * (1 - ty) * p00.g + tx * (1 - ty) * p10.g +
                    (1 - tx) * ty * p01.g + tx * ty * p11.g
                );
                interpolated.b = static_cast<unsigned char>(
                    (1 - tx) * (1 - ty) * p00.b + tx * (1 - ty) * p10.b +
                    (1 - tx) * ty * p01.b + tx * ty * p11.b
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

    void printTopLeft(int limit = 5) const {
        for (int y = 0; y < std::min(limit, height); ++y) {
            for (int x = 0; x < std::min(limit, width); ++x) {
                const Pixel& p = at(x, y);
                std::cout << "(" << static_cast<int>(p.r) << ","
                          << static_cast<int>(p.g) << ","
                          << static_cast<int>(p.b) << ") ";
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    try {
        Image original(8, 6);
        original.fillTestPattern();
        std::cout << "Original image top-left pixels:" << std::endl;
        original.printTopLeft();

        Image resized = original.resize(12, 9);
        std::cout << "\nResized image top-left pixels:" << std::endl;
        resized.printTopLeft();

        std::cout << "\nOriginal dimensions: " << original.getWidth()
                  << "x" << original.getHeight() << std::endl;
        std::cout << "Resized dimensions: " << resized.getWidth()
                  << "x" << resized.getHeight() << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}