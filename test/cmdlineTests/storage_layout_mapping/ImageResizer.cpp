#include <iostream>
#include <vector>
#include <cmath>
#include <cstdint>

struct Pixel {
    uint8_t r, g, b;
};

class Image {
private:
    int width, height;
    std::vector<Pixel> data;

public:
    Image(int w, int h) : width(w), height(h), data(w * h) {}

    Pixel& at(int x, int y) {
        return data[y * width + x];
    }

    const Pixel& at(int x, int y) const {
        return data[y * width + x];
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    void fill(const Pixel& color) {
        std::fill(data.begin(), data.end(), color);
    }
};

Image resizeImage(const Image& src, int newWidth, int newHeight) {
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

            const Pixel& p00 = src.at(gxi, gyi);
            const Pixel& p10 = src.at(std::min(gxi + 1, src.getWidth() - 1), gyi);
            const Pixel& p01 = src.at(gxi, std::min(gyi + 1, src.getHeight() - 1));
            const Pixel& p11 = src.at(std::min(gxi + 1, src.getWidth() - 1), std::min(gyi + 1, src.getHeight() - 1));

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

            dst.at(x, y) = interpolated;
        }
    }
    return dst;
}

void printImageInfo(const Image& img) {
    std::cout << "Image size: " << img.getWidth() << "x" << img.getHeight() << std::endl;
    if (img.getWidth() > 0 && img.getHeight() > 0) {
        Pixel p = img.at(0, 0);
        std::cout << "Top-left pixel color: RGB("
                  << static_cast<int>(p.r) << ", "
                  << static_cast<int>(p.g) << ", "
                  << static_cast<int>(p.b) << ")" << std::endl;
    }
}

int main() {
    const int srcWidth = 4;
    const int srcHeight = 4;
    Image source(srcWidth, srcHeight);

    for (int y = 0; y < srcHeight; ++y) {
        for (int x = 0; x < srcWidth; ++x) {
            Pixel p;
            p.r = static_cast<uint8_t>((x * 64) % 256);
            p.g = static_cast<uint8_t>((y * 64) % 256);
            p.b = static_cast<uint8_t>(((x + y) * 32) % 256);
            source.at(x, y) = p;
        }
    }

    std::cout << "Original image:" << std::endl;
    printImageInfo(source);

    const int dstWidth = 8;
    const int dstHeight = 8;
    Image resized = resizeImage(source, dstWidth, dstHeight);

    std::cout << "\nResized image:" << std::endl;
    printImageInfo(resized);

    return 0;
}