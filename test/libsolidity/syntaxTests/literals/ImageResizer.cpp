
#include <vector>
#include <stdexcept>
#include <cmath>

class ImageResizer {
public:
    static std::vector<std::vector<double>> resize(const std::vector<std::vector<double>>& source,
                                                   int newWidth, int newHeight) {
        if (source.empty() || source[0].empty()) {
            throw std::invalid_argument("Source image cannot be empty");
        }
        if (newWidth <= 0 || newHeight <= 0) {
            throw std::invalid_argument("New dimensions must be positive");
        }

        int oldHeight = source.size();
        int oldWidth = source[0].size();

        std::vector<std::vector<double>> result(newHeight, std::vector<double>(newWidth, 0.0));

        double xRatio = static_cast<double>(oldWidth - 1) / newWidth;
        double yRatio = static_cast<double>(oldHeight - 1) / newHeight;

        for (int y = 0; y < newHeight; ++y) {
            for (int x = 0; x < newWidth; ++x) {
                double gx = x * xRatio;
                double gy = y * yRatio;

                int x1 = static_cast<int>(gx);
                int y1 = static_cast<int>(gy);
                int x2 = std::min(x1 + 1, oldWidth - 1);
                int y2 = std::min(y1 + 1, oldHeight - 1);

                double dx = gx - x1;
                double dy = gy - y1;

                double value = (1 - dx) * (1 - dy) * source[y1][x1] +
                               dx * (1 - dy) * source[y1][x2] +
                               (1 - dx) * dy * source[y2][x1] +
                               dx * dy * source[y2][x2];

                result[y][x] = value;
            }
        }

        return result;
    }

    static void normalizeImage(std::vector<std::vector<double>>& image) {
        if (image.empty()) return;

        double minVal = image[0][0];
        double maxVal = image[0][0];

        for (const auto& row : image) {
            for (double val : row) {
                if (val < minVal) minVal = val;
                if (val > maxVal) maxVal = val;
            }
        }

        double range = maxVal - minVal;
        if (range == 0) return;

        for (auto& row : image) {
            for (double& val : row) {
                val = (val - minVal) / range;
            }
        }
    }
};