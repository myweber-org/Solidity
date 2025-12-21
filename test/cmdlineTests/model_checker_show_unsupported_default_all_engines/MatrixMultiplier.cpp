
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <omp.h>

std::vector<std::vector<double>> generateRandomMatrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 10.0);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = dis(gen);
        }
    }
    return matrix;
}

std::vector<std::vector<double>> multiplyMatrices(const std::vector<std::vector<double>>& A,
                                                  const std::vector<std::vector<double>>& B) {
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();
    std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));

    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            double sum = 0.0;
            for (int k = 0; k < colsA; ++k) {
                sum += A[i][k] * B[k][j];
            }
            result[i][j] = sum;
        }
    }
    return result;
}

void printMatrix(const std::vector<std::vector<double>>& matrix) {
    for (const auto& row : matrix) {
        for (double val : row) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    const int N = 500;
    const int M = 500;
    const int P = 500;

    auto start = std::chrono::high_resolution_clock::now();
    auto matrixA = generateRandomMatrix(N, M);
    auto matrixB = generateRandomMatrix(M, P);
    auto genEnd = std::chrono::high_resolution_clock::now();

    auto multiplyStart = std::chrono::high_resolution_clock::now();
    auto result = multiplyMatrices(matrixA, matrixB);
    auto multiplyEnd = std::chrono::high_resolution_clock::now();

    auto genDuration = std::chrono::duration_cast<std::chrono::milliseconds>(genEnd - start);
    auto multiplyDuration = std::chrono::duration_cast<std::chrono::milliseconds>(multiplyEnd - multiplyStart);

    std::cout << "Matrix generation time: " << genDuration.count() << " ms" << std::endl;
    std::cout << "Matrix multiplication time: " << multiplyDuration.count() << " ms" << std::endl;
    std::cout << "Total execution time: " << (genDuration + multiplyDuration).count() << " ms" << std::endl;

    if (N <= 5 && M <= 5 && P <= 5) {
        std::cout << "\nMatrix A:" << std::endl;
        printMatrix(matrixA);
        std::cout << "\nMatrix B:" << std::endl;
        printMatrix(matrixB);
        std::cout << "\nResult matrix:" << std::endl;
        printMatrix(result);
    }

    return 0;
}