
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
        if (x < 0 || x >= width || y < 0 || y >= height) {
            throw std::out_of_range("Pixel coordinates out of bounds");
        }
        return data[y][x];
    }

    void setPixel(int x, int y, const Pixel& p) {
        if (x < 0 || x >= width || y < 0 || y >= height) {
            throw std::out_of_range("Pixel coordinates out of bounds");
        }
        data[y][x] = p;
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
                Pixel p10 = getPixel(std::min(gxi + 1, width - 1), gyi);
                Pixel p01 = getPixel(gxi, std::min(gyi + 1, height - 1));
                Pixel p11 = getPixel(std::min(gxi + 1, width - 1), std::min(gyi + 1, height - 1));

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

                resized.setPixel(x, y, interpolated);
            }
        }
        return resized;
    }

    void fillWithGradient() {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                Pixel p;
                p.r = static_cast<unsigned char>((255.0 * x) / width);
                p.g = static_cast<unsigned char>((255.0 * y) / height);
                p.b = static_cast<unsigned char>(128 + 127.0 * sin(x * 0.1) * cos(y * 0.1));
                setPixel(x, y, p);
            }
        }
    }

    void printInfo() const {
        std::cout << "Image dimensions: " << width << "x" << height << std::endl;
        if (width > 0 && height > 0) {
            Pixel sample = getPixel(0, 0);
            std::cout << "Top-left pixel RGB: (" 
                      << static_cast<int>(sample.r) << ", " 
                      << static_cast<int>(sample.g) << ", " 
                      << static_cast<int>(sample.b) << ")" << std::endl;
        }
    }
};

int main() {
    try {
        Image original(800, 600);
        original.fillWithGradient();
        std::cout << "Original image:" << std::endl;
        original.printInfo();

        Image resized = original.resize(400, 300);
        std::cout << "\nResized image:" << std::endl;
        resized.printInfo();

        Image small = original.resize(100, 75);
        std::cout << "\nSmall preview:" << std::endl;
        small.printInfo();

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}