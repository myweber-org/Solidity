
#include <vector>
#include <stdexcept>
#include <cmath>

class ImageResizer {
public:
    static std::vector<std::vector<double>> resizeBilinear(const std::vector<std::vector<double>>& input,
                                                          int newHeight, int newWidth) {
        if (input.empty() || input[0].empty()) {
            throw std::invalid_argument("Input image cannot be empty");
        }
        if (newHeight <= 0 || newWidth <= 0) {
            throw std::invalid_argument("New dimensions must be positive");
        }

        int oldHeight = static_cast<int>(input.size());
        int oldWidth = static_cast<int>(input[0].size());

        std::vector<std::vector<double>> output(newHeight, std::vector<double>(newWidth, 0.0));

        double xRatio = static_cast<double>(oldWidth - 1) / (newWidth - 1);
        double yRatio = static_cast<double>(oldHeight - 1) / (newHeight - 1);

        for (int y = 0; y < newHeight; ++y) {
            for (int x = 0; x < newWidth; ++x) {
                double gx = x * xRatio;
                double gy = y * yRatio;

                int x1 = static_cast<int>(std::floor(gx));
                int y1 = static_cast<int>(std::floor(gy));
                int x2 = std::min(x1 + 1, oldWidth - 1);
                int y2 = std::min(y1 + 1, oldHeight - 1);

                double dx = gx - x1;
                double dy = gy - y1;

                double value = (1 - dx) * (1 - dy) * input[y1][x1] +
                              dx * (1 - dy) * input[y1][x2] +
                              (1 - dx) * dy * input[y2][x1] +
                              dx * dy * input[y2][x2];

                output[y][x] = value;
            }
        }

        return output;
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