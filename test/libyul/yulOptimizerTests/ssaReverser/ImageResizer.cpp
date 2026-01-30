
#include <vector>
#include <stdexcept>
#include <cmath>

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

                double top = interpolate(source[yFloor][xFloor], source[yFloor][xCeil], xFraction);
                double bottom = interpolate(source[yCeil][xFloor], source[yCeil][xCeil], xFraction);

                result[y][x] = interpolate(top, bottom, yFraction);
            }
        }

        return result;
    }

private:
    static double interpolate(double a, double b, double fraction) {
        return a + fraction * (b - a);
    }
};