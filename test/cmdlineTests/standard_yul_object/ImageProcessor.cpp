#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>

struct Pixel {
    unsigned char r, g, b;
};

class Image {
private:
    int width;
    int height;
    std::vector<Pixel> pixels;

public:
    Image(int w, int h) : width(w), height(h), pixels(w * h) {}

    bool loadFromPPM(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return false;
        }

        std::string format;
        file >> format;
        if (format != "P6") {
            std::cerr << "Error: Unsupported PPM format. Only P6 is supported." << std::endl;
            return false;
        }

        file >> width >> height;
        int maxVal;
        file >> maxVal;
        file.ignore(1, '\n');

        pixels.resize(width * height);
        file.read(reinterpret_cast<char*>(pixels.data()), width * height * sizeof(Pixel));

        return file.good();
    }

    bool saveToPPM(const std::string& filename) const {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error: Could not create file " << filename << std::endl;
            return false;
        }

        file << "P6\n" << width << " " << height << "\n255\n";
        file.write(reinterpret_cast<const char*>(pixels.data()), width * height * sizeof(Pixel));

        return file.good();
    }

    void applyGrayscale() {
        for (auto& pixel : pixels) {
            unsigned char gray = static_cast<unsigned char>(0.299 * pixel.r + 0.587 * pixel.g + 0.114 * pixel.b);
            pixel.r = pixel.g = pixel.b = gray;
        }
    }

    void invertColors() {
        for (auto& pixel : pixels) {
            pixel.r = 255 - pixel.r;
            pixel.g = 255 - pixel.g;
            pixel.b = 255 - pixel.b;
        }
    }

    Pixel& at(int x, int y) {
        if (x < 0 || x >= width || y < 0 || y >= height) {
            throw std::out_of_range("Pixel coordinates out of range");
        }
        return pixels[y * width + x];
    }

    const Pixel& at(int x, int y) const {
        if (x < 0 || x >= width || y < 0 || y >= height) {
            throw std::out_of_range("Pixel coordinates out of range");
        }
        return pixels[y * width + x];
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }
};

void processImage(const std::string& inputFile, const std::string& outputFile) {
    Image img(0, 0);
    if (!img.loadFromPPM(inputFile)) {
        std::cerr << "Failed to load image." << std::endl;
        return;
    }

    std::cout << "Image loaded successfully. Dimensions: "
              << img.getWidth() << "x" << img.getHeight() << std::endl;

    img.applyGrayscale();
    img.invertColors();

    if (img.saveToPPM(outputFile)) {
        std::cout << "Processed image saved to " << outputFile << std::endl;
    } else {
        std::cerr << "Failed to save image." << std::endl;
    }
}

int main() {
    std::string inputFilename = "input.ppm";
    std::string outputFilename = "output.ppm";

    processImage(inputFilename, outputFilename);
    return 0;
}