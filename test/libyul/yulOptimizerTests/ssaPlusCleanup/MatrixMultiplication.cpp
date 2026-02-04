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

std::vector<std::vector<double>> multiplyMatricesParallel(
    const std::vector<std::vector<double>>& A,
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

void printMatrixStats(const std::vector<std::vector<double>>& matrix) {
    double sum = 0.0;
    double minVal = matrix[0][0];
    double maxVal = matrix[0][0];
    
    for (const auto& row : matrix) {
        for (double val : row) {
            sum += val;
            if (val < minVal) minVal = val;
            if (val > maxVal) maxVal = val;
        }
    }
    
    std::cout << "Matrix statistics:" << std::endl;
    std::cout << "  Sum of all elements: " << sum << std::endl;
    std::cout << "  Minimum value: " << minVal << std::endl;
    std::cout << "  Maximum value: " << maxVal << std::endl;
    std::cout << "  Average value: " << sum / (matrix.size() * matrix[0].size()) << std::endl;
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    const int N = 500;
    const int M = 500;
    const int P = 500;
    
    std::cout << "Generating random matrices..." << std::endl;
    auto matrixA = generateRandomMatrix(N, M);
    auto matrixB = generateRandomMatrix(M, P);
    
    std::cout << "Performing parallel matrix multiplication..." << std::endl;
    double startTime = omp_get_wtime();
    
    auto result = multiplyMatricesParallel(matrixA, matrixB);
    
    double endTime = omp_get_wtime();
    double elapsedTime = endTime - startTime;
    
    std::cout << "Multiplication completed in " << elapsedTime << " seconds" << std::endl;
    std::cout << "Performance: " << (2.0 * N * M * P) / (elapsedTime * 1e9) << " GFLOPS" << std::endl;
    
    printMatrixStats(result);
    
    return 0;
}
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

void initializeMatrix(std::vector<std::vector<double>>& matrix, int size) {
    matrix.resize(size, std::vector<double>(size));
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
        }
    }
}

void multiplyMatrices(const std::vector<std::vector<double>>& A,
                      const std::vector<std::vector<double>>& B,
                      std::vector<std::vector<double>>& C,
                      int size) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            C[i][j] = 0.0;
            for (int k = 0; k < size; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

int main() {
    const int MATRIX_SIZE = 500;
    srand(static_cast<unsigned>(time(nullptr)));

    std::vector<std::vector<double>> A, B, C;
    
    std::cout << "Initializing matrices..." << std::endl;
    initializeMatrix(A, MATRIX_SIZE);
    initializeMatrix(B, MATRIX_SIZE);
    C.resize(MATRIX_SIZE, std::vector<double>(MATRIX_SIZE));

    std::cout << "Performing matrix multiplication..." << std::endl;
    double start_time = omp_get_wtime();
    
    multiplyMatrices(A, B, C, MATRIX_SIZE);
    
    double end_time = omp_get_wtime();
    std::cout << "Multiplication completed in " << (end_time - start_time) 
              << " seconds" << std::endl;

    return 0;
}