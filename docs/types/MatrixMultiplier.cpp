#include <vector>
#include <stdexcept>

std::vector<std::vector<double>> multiplyMatrices(const std::vector<std::vector<double>>& A,
                                                  const std::vector<std::vector<double>>& B) {
    if (A.empty() || B.empty()) {
        throw std::invalid_argument("Input matrices cannot be empty.");
    }
    size_t rowsA = A.size();
    size_t colsA = A[0].size();
    size_t rowsB = B.size();
    size_t colsB = B[0].size();

    for (const auto& row : A) {
        if (row.size() != colsA) {
            throw std::invalid_argument("Matrix A is not rectangular.");
        }
    }
    for (const auto& row : B) {
        if (row.size() != colsB) {
            throw std::invalid_argument("Matrix B is not rectangular.");
        }
    }

    if (colsA != rowsB) {
        throw std::invalid_argument("Matrix dimensions are incompatible for multiplication.");
    }

    std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));

    for (size_t i = 0; i < rowsA; ++i) {
        for (size_t j = 0; j < colsB; ++j) {
            double sum = 0.0;
            for (size_t k = 0; k < colsA; ++k) {
                sum += A[i][k] * B[k][j];
            }
            result[i][j] = sum;
        }
    }

    return result;
}