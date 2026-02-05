
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <omp.h>

std::vector<std::vector<double>> generate_random_matrix(int rows, int cols) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 100.0);
    
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = dis(gen);
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

bool verify_results(const std::vector<std::vector<double>>& result1,
                    const std::vector<std::vector<double>>& result2,
                    double tolerance = 1e-6) {
    
    if (result1.size() != result2.size() || result1[0].size() != result2[0].size()) {
        return false;
    }
    
    for (size_t i = 0; i < result1.size(); ++i) {
        for (size_t j = 0; j < result1[0].size(); ++j) {
            if (std::abs(result1[i][j] - result2[i][j]) > tolerance) {
                return false;
            }
        }
    }
    return true;
}

int main() {
    const int MATRIX_SIZE = 500;
    
    std::cout << "Generating random matrices of size " << MATRIX_SIZE << "x" << MATRIX_SIZE << std::endl;
    auto matrix_A = generate_random_matrix(MATRIX_SIZE, MATRIX_SIZE);
    auto matrix_B = generate_random_matrix(MATRIX_SIZE, MATRIX_SIZE);
    
    std::cout << "Starting parallel matrix multiplication..." << std::endl;
    auto start_parallel = std::chrono::high_resolution_clock::now();
    auto result_parallel = multiply_matrices_parallel(matrix_A, matrix_B);
    auto end_parallel = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> parallel_duration = end_parallel - start_parallel;
    
    std::cout << "Starting sequential matrix multiplication..." << std::endl;
    auto start_sequential = std::chrono::high_resolution_clock::now();
    auto result_sequential = multiply_matrices_sequential(matrix_A, matrix_B);
    auto end_sequential = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> sequential_duration = end_sequential - start_sequential;
    
    std::cout << "\nPerformance Results:" << std::endl;
    std::cout << "Parallel execution time: " << parallel_duration.count() << " seconds" << std::endl;
    std::cout << "Sequential execution time: " << sequential_duration.count() << " seconds" << std::endl;
    std::cout << "Speedup factor: " << sequential_duration.count() / parallel_duration.count() << std::endl;
    
    std::cout << "\nVerifying results..." << std::endl;
    if (verify_results(result_parallel, result_sequential)) {
        std::cout << "Results match! Parallel computation is correct." << std::endl;
    } else {
        std::cout << "ERROR: Results do not match!" << std::endl;
    }
    
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
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX * 100.0;
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

void printMatrixStats(const std::vector<std::vector<double>>& matrix, const std::string& name) {
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
    
    std::cout << name << " - Sum: " << sum 
              << ", Min: " << minVal 
              << ", Max: " << maxVal 
              << ", Avg: " << sum / (matrix.size() * matrix[0].size()) 
              << std::endl;
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    const int SIZE = 500;
    
    std::cout << "Generating random matrices of size " << SIZE << "x" << SIZE << "..." << std::endl;
    auto matrixA = generateRandomMatrix(SIZE, SIZE);
    auto matrixB = generateRandomMatrix(SIZE, SIZE);
    
    std::cout << "Performing parallel matrix multiplication..." << std::endl;
    double startTime = omp_get_wtime();
    
    auto result = multiplyMatricesParallel(matrixA, matrixB);
    
    double endTime = omp_get_wtime();
    double elapsedTime = endTime - startTime;
    
    std::cout << "Matrix multiplication completed in " << elapsedTime << " seconds" << std::endl;
    std::cout << "Performance: " << (2.0 * SIZE * SIZE * SIZE) / (elapsedTime * 1e9) << " GFLOPS" << std::endl;
    
    printMatrixStats(matrixA, "Matrix A");
    printMatrixStats(matrixB, "Matrix B");
    printMatrixStats(result, "Result Matrix");
    
    return 0;
}