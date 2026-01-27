
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
    int cols_B = B[0].size();
    
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

std::vector<std::vector<double>> multiply_matrices_sequential(
    const std::vector<std::vector<double>>& A,
    const std::vector<std::vector<double>>& B) {
    
    int rows_A = A.size();
    int cols_A = A[0].size();
    int cols_B = B[0].size();
    
    std::vector<std::vector<double>> result(rows_A, std::vector<double>(cols_B, 0.0));
    
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

bool compare_matrices(const std::vector<std::vector<double>>& A,
                     const std::vector<std::vector<double>>& B,
                     double tolerance = 1e-6) {
    if (A.size() != B.size() || A[0].size() != B[0].size()) {
        return false;
    }
    
    for (size_t i = 0; i < A.size(); ++i) {
        for (size_t j = 0; j < A[0].size(); ++j) {
            if (std::abs(A[i][j] - B[i][j]) > tolerance) {
                return false;
            }
        }
    }
    return true;
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    const int SIZE = 500;
    
    std::cout << "Generating random " << SIZE << "x" << SIZE << " matrices..." << std::endl;
    auto matrix_A = generate_random_matrix(SIZE, SIZE);
    auto matrix_B = generate_random_matrix(SIZE, SIZE);
    
    std::cout << "Performing sequential multiplication..." << std::endl;
    double start_seq = omp_get_wtime();
    auto result_seq = multiply_matrices_sequential(matrix_A, matrix_B);
    double end_seq = omp_get_wtime();
    
    std::cout << "Performing parallel multiplication..." << std::endl;
    double start_par = omp_get_wtime();
    auto result_par = multiply_matrices_parallel(matrix_A, matrix_B);
    double end_par = omp_get_wtime();
    
    std::cout << "\nResults verification: ";
    if (compare_matrices(result_seq, result_par)) {
        std::cout << "PASSED" << std::endl;
    } else {
        std::cout << "FAILED" << std::endl;
        return 1;
    }
    
    std::cout << "\nPerformance comparison:" << std::endl;
    std::cout << "Sequential time: " << (end_seq - start_seq) << " seconds" << std::endl;
    std::cout << "Parallel time: " << (end_par - start_par) << " seconds" << std::endl;
    std::cout << "Speedup: " << (end_seq - start_seq) / (end_par - start_par) << "x" << std::endl;
    
    return 0;
}
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
    const int M = 500;
    const int P = 500;
    
    std::vector<std::vector<double>> matrixA = generateRandomMatrix(N, M);
    std::vector<std::vector<double>> matrixB = generateRandomMatrix(M, P);
    
    double startTime = omp_get_wtime();
    std::vector<std::vector<double>> result = multiplyMatrices(matrixA, matrixB);
    double endTime = omp_get_wtime();
    
    std::cout << "Matrix multiplication completed." << std::endl;
    std::cout << "Matrix A: " << N << " x " << M << std::endl;
    std::cout << "Matrix B: " << M << " x " << P << std::endl;
    std::cout << "Result matrix: " << N << " x " << P << std::endl;
    std::cout << "Execution time: " << (endTime - startTime) << " seconds" << std::endl;
    
    return 0;
}