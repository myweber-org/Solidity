
#include <iostream>
#include <vector>
#include <cmath>
#include <stdexcept>

class Image {
private:
    std::vector<std::vector<std::vector<double>>> data; // [height][width][channels]
    int height, width, channels;

public:
    Image(int h, int w, int c) : height(h), width(w), channels(c) {
        data.resize(height, std::vector<std::vector<double>>(width, std::vector<double>(channels, 0.0)));
    }

    void setPixel(int y, int x, int c, double value) {
        if (y >= 0 && y < height && x >= 0 && x < width && c >= 0 && c < channels) {
            data[y][x][c] = value;
        }
    }

    double getPixel(int y, int x, int c) const {
        if (y >= 0 && y < height && x >= 0 && x < width && c >= 0 && c < channels) {
            return data[y][x][c];
        }
        return 0.0;
    }

    int getHeight() const { return height; }
    int getWidth() const { return width; }
    int getChannels() const { return channels; }

    Image resize(int newHeight, int newWidth) const {
        Image resized(newHeight, newWidth, channels);
        double scaleY = static_cast<double>(height) / newHeight;
        double scaleX = static_cast<double>(width) / newWidth;

        for (int y = 0; y < newHeight; ++y) {
            for (int x = 0; x < newWidth; ++x) {
                double srcY = y * scaleY;
                double srcX = x * scaleX;
                int y0 = static_cast<int>(std::floor(srcY));
                int x0 = static_cast<int>(std::floor(srcX));
                int y1 = std::min(y0 + 1, height - 1);
                int x1 = std::min(x0 + 1, width - 1);

                double dy = srcY - y0;
                double dx = srcX - x0;

                for (int c = 0; c < channels; ++c) {
                    double interpolated =
                        (1 - dx) * (1 - dy) * getPixel(y0, x0, c) +
                        dx * (1 - dy) * getPixel(y0, x1, c) +
                        (1 - dx) * dy * getPixel(y1, x0, c) +
                        dx * dy * getPixel(y1, x1, c);
                    resized.setPixel(y, x, c, interpolated);
                }
            }
        }
        return resized;
    }

    void printChannel(int c) const {
        if (c < 0 || c >= channels) throw std::out_of_range("Channel index out of range");
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                std::cout << data[y][x][c] << " ";
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    Image img(4, 4, 3);
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            img.setPixel(y, x, 0, y * 4 + x);
            img.setPixel(y, x, 1, (y * 4 + x) * 2);
            img.setPixel(y, x, 2, (y * 4 + x) * 3);
        }
    }

    std::cout << "Original image channel 0:" << std::endl;
    img.printChannel(0);

    Image resized = img.resize(2, 2);
    std::cout << "\nResized image channel 0:" << std::endl;
    resized.printChannel(0);

    return 0;
}