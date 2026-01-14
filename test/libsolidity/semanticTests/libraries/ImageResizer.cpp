
#include <vector>
#include <cmath>
#include <stdexcept>

class ImageResizer {
public:
    static std::vector<std::vector<double>> resizeBilinear(const std::vector<std::vector<double>>& input,
                                                           int newHeight, int newWidth) {
        int oldHeight = input.size();
        if (oldHeight == 0) throw std::invalid_argument("Input image height is zero");
        int oldWidth = input[0].size();
        if (oldWidth == 0) throw std::invalid_argument("Input image width is zero");

        std::vector<std::vector<double>> output(newHeight, std::vector<double>(newWidth, 0.0));

        double xRatio = static_cast<double>(oldWidth - 1) / (newWidth - 1);
        double yRatio = static_cast<double>(oldHeight - 1) / (newHeight - 1);

        for (int y = 0; y < newHeight; ++y) {
            for (int x = 0; x < newWidth; ++x) {
                double gx = x * xRatio;
                double gy = y * yRatio;
                int gxi = static_cast<int>(gx);
                int gyi = static_cast<int>(gy);

                double dx = gx - gxi;
                double dy = gy - gyi;

                double a00 = input[gyi][gxi];
                double a01 = (gxi + 1 < oldWidth) ? input[gyi][gxi + 1] : a00;
                double a10 = (gyi + 1 < oldHeight) ? input[gyi + 1][gxi] : a00;
                double a11 = ((gxi + 1 < oldWidth) && (gyi + 1 < oldHeight)) ? input[gyi + 1][gxi + 1] : a00;

                double value = a00 * (1 - dx) * (1 - dy) +
                               a01 * dx * (1 - dy) +
                               a10 * (1 - dx) * dy +
                               a11 * dx * dy;

                output[y][x] = value;
            }
        }

        return output;
    }
};