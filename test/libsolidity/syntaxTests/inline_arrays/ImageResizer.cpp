
#include <vector>
#include <cmath>
#include <stdexcept>

class ImageResizer {
public:
    static std::vector<std::vector<double>> resizeBilinear(const std::vector<std::vector<double>>& input, int newWidth, int newHeight) {
        if (input.empty() || input[0].empty()) {
            throw std::invalid_argument("Input image cannot be empty");
        }
        if (newWidth <= 0 || newHeight <= 0) {
            throw std::invalid_argument("New dimensions must be positive");
        }

        int oldHeight = input.size();
        int oldWidth = input[0].size();

        std::vector<std::vector<double>> output(newHeight, std::vector<double>(newWidth, 0.0));

        double xRatio = static_cast<double>(oldWidth - 1) / newWidth;
        double yRatio = static_cast<double>(oldHeight - 1) / newHeight;

        for (int y = 0; y < newHeight; ++y) {
            for (int x = 0; x < newWidth; ++x) {
                double gx = x * xRatio;
                double gy = y * yRatio;
                int gxi = static_cast<int>(gx);
                int gyi = static_cast<int>(gy);

                double dx = gx - gxi;
                double dy = gy - gyi;

                double c00 = input[gyi][gxi];
                double c10 = (gxi + 1 < oldWidth) ? input[gyi][gxi + 1] : c00;
                double c01 = (gyi + 1 < oldHeight) ? input[gyi + 1][gxi] : c00;
                double c11 = (gxi + 1 < oldWidth && gyi + 1 < oldHeight) ? input[gyi + 1][gxi + 1] : c00;

                double top = c00 * (1 - dx) + c10 * dx;
                double bottom = c01 * (1 - dx) + c11 * dx;
                output[y][x] = top * (1 - dy) + bottom * dy;
            }
        }

        return output;
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