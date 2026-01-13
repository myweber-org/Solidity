
#include <vector>
#include <cmath>
#include <stdexcept>

class ImageResizer {
public:
    static std::vector<std::vector<double>> resizeBilinear(const std::vector<std::vector<double>>& source, int newWidth, int newHeight) {
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
                double gx = x * xRatio;
                double gy = y * yRatio;
                int gxi = static_cast<int>(gx);
                int gyi = static_cast<int>(gy);

                double dx = gx - gxi;
                double dy = gy - gyi;

                double c00 = source[gyi][gxi];
                double c10 = (gxi + 1 < srcWidth) ? source[gyi][gxi + 1] : c00;
                double c01 = (gyi + 1 < srcHeight) ? source[gyi + 1][gxi] : c00;
                double c11 = (gxi + 1 < srcWidth && gyi + 1 < srcHeight) ? source[gyi + 1][gxi + 1] : c00;

                double top = c00 * (1 - dx) + c10 * dx;
                double bottom = c01 * (1 - dx) + c11 * dx;
                result[y][x] = top * (1 - dy) + bottom * dy;
            }
        }

        return result;
    }
};