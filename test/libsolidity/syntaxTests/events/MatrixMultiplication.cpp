#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

std::vector<std::vector<double>> generateRandomMatrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
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

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    const int N = 500;

    std::vector<std::vector<double>> matrixA = generateRandomMatrix(N, N);
    std::vector<std::vector<double>> matrixB = generateRandomMatrix(N, N);

    double startTime = omp_get_wtime();
    std::vector<std::vector<double>> result = multiplyMatrices(matrixA, matrixB);
    double endTime = omp_get_wtime();

    std::cout << "Matrix multiplication of size " << N << "x" << N << " completed." << std::endl;
    std::cout << "Execution time: " << endTime - startTime << " seconds" << std::endl;

    return 0;
}