
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

    std::cout << "Matrix multiplication of size " << N << "x" << N << " completed.\n";
    std::cout << "Execution time: " << endTime - startTime << " seconds.\n";

    return 0;
}
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

std::vector<std::vector<double>> generate_random_matrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX * 10.0;
        }
    }
    return matrix;
}

std::vector<std::vector<double>> multiply_matrices_parallel(
    const std::vector<std::vector<double>>& A,
    const std::vector<std::vector<double>>& B) {
    
    int rows_A = A.size();
    int cols_A = A[0].size();
    int rows_B = B.size();
    int cols_B = B[0].size();
    
    if (cols_A != rows_B) {
        throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
    }
    
    std::vector<std::vector<double>> result(rows_A, std::vector<double>(cols_B, 0.0));
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rows_A; ++i) {
        for (int j = 0; j < cols_B; ++j) {
            double sum = 0.0;
            for (int k = 0; k < cols_A; ++k) {
                sum += A[i][k] * B[k][j];
            }
            result[i][j] = sum;
        }
    }
    
    return result;
}

void print_matrix(const std::vector<std::vector<double>>& matrix, int max_rows = 5, int max_cols = 5) {
    int rows = std::min(static_cast<int>(matrix.size()), max_rows);
    int cols = std::min(static_cast<int>(matrix[0].size()), max_cols);
    
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            std::cout << matrix[i][j] << " ";
        }
        std::cout << "...\n";
    }
    std::cout << "...\n";
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    const int N = 1000;
    const int M = 800;
    const int P = 1200;
    
    std::cout << "Generating random matrices: " << N << "x" << M << " and " << M << "x" << P << "\n";
    
    auto matrix_A = generate_random_matrix(N, M);
    auto matrix_B = generate_random_matrix(M, P);
    
    std::cout << "Starting parallel matrix multiplication...\n";
    double start_time = omp_get_wtime();
    
    auto result = multiply_matrices_parallel(matrix_A, matrix_B);
    
    double end_time = omp_get_wtime();
    std::cout << "Multiplication completed in " << (end_time - start_time) << " seconds\n";
    
    std::cout << "\nFirst 5x5 elements of result matrix:\n";
    print_matrix(result);
    
    return 0;
}