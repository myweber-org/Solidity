
#include <vector>
#include <cmath>
#include <stdexcept>

class ImageResizer {
public:
    static std::vector<std::vector<double>> resizeBilinear(const std::vector<std::vector<double>>& source,
                                                           int newWidth, int newHeight) {
        if (source.empty() || source[0].empty()) {
            throw std::invalid_argument("Source image cannot be empty");
        }
        if (newWidth <= 0 || newHeight <= 0) {
            throw std::invalid_argument("New dimensions must be positive");
        }

        int srcHeight = source.size();
        int srcWidth = source[0].size();

        std::vector<std::vector<double>> result(newHeight, std::vector<double>(newWidth, 0.0));

        double xRatio = static_cast<double>(srcWidth - 1) / newWidth;
        double yRatio = static_cast<double>(srcHeight - 1) / newHeight;

        for (int y = 0; y < newHeight; ++y) {
            for (int x = 0; x < newWidth; ++x) {
                double srcX = x * xRatio;
                double srcY = y * yRatio;

                int xFloor = static_cast<int>(srcX);
                int yFloor = static_cast<int>(srcY);
                int xCeil = std::min(xFloor + 1, srcWidth - 1);
                int yCeil = std::min(yFloor + 1, srcHeight - 1);

                double xFraction = srcX - xFloor;
                double yFraction = srcY - yFloor;

                double topInterpolation = source[yFloor][xFloor] * (1 - xFraction) + source[yFloor][xCeil] * xFraction;
                double bottomInterpolation = source[yCeil][xFloor] * (1 - xFraction) + source[yCeil][xCeil] * xFraction;

                result[y][x] = topInterpolation * (1 - yFraction) + bottomInterpolation * yFraction;
            }
        }

        return result;
    }

    static void clampValues(std::vector<std::vector<double>>& image, double minVal, double maxVal) {
        for (auto& row : image) {
            for (auto& pixel : row) {
                if (pixel < minVal) pixel = minVal;
                if (pixel > maxVal) pixel = maxVal;
            }
        }
    }
};