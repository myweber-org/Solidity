
#include <vector>
#include <cmath>
#include <stdexcept>

class ImageResizer {
public:
    static std::vector<std::vector<double>> resizeBilinear(const std::vector<std::vector<double>>& src, int newWidth, int newHeight) {
        if (src.empty() || src[0].empty()) {
            throw std::invalid_argument("Source image cannot be empty");
        }
        if (newWidth <= 0 || newHeight <= 0) {
            throw std::invalid_argument("New dimensions must be positive");
        }

        int srcHeight = src.size();
        int srcWidth = src[0].size();
        std::vector<std::vector<double>> dst(newHeight, std::vector<double>(newWidth, 0.0));

        double xRatio = static_cast<double>(srcWidth - 1) / newWidth;
        double yRatio = static_cast<double>(srcHeight - 1) / newHeight;

        for (int y = 0; y < newHeight; ++y) {
            for (int x = 0; x < newWidth; ++x) {
                double gx = x * xRatio;
                double gy = y * yRatio;
                int gxi = static_cast<int>(gx);
                int gyi = static_cast<int>(gy);

                double dx = gx - gxi;
                double dy = gy - gyi;

                double a = src[gyi][gxi];
                double b = (gxi + 1 < srcWidth) ? src[gyi][gxi + 1] : a;
                double c = (gyi + 1 < srcHeight) ? src[gyi + 1][gxi] : a;
                double d = (gxi + 1 < srcWidth && gyi + 1 < srcHeight) ? src[gyi + 1][gxi + 1] : a;

                double value = a * (1 - dx) * (1 - dy) +
                               b * dx * (1 - dy) +
                               c * (1 - dx) * dy +
                               d * dx * dy;
                dst[y][x] = value;
            }
        }
        return dst;
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
        if (range == 0.0) return;
        
        for (auto& row : image) {
            for (double& val : row) {
                val = (val - minVal) / range;
            }
        }
    }
};