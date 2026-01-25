
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

class ParallelMatrixMultiplier {
public:
    static std::vector<std::vector<double>> multiply(const std::vector<std::vector<double>>& A,
                                                     const std::vector<std::vector<double>>& B) {
        size_t rowsA = A.size();
        size_t colsA = A[0].size();
        size_t colsB = B[0].size();
        
        std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));
        
        #pragma omp parallel for collapse(2)
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
    
    static std::vector<std::vector<double>> generateRandomMatrix(size_t rows, size_t cols) {
        std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                matrix[i][j] = static_cast<double>(std::rand()) / RAND_MAX * 100.0;
            }
        }
        return matrix;
    }
    
    static void printMatrix(const std::vector<std::vector<double>>& matrix, size_t maxRows = 5, size_t maxCols = 5) {
        size_t rows = std::min(matrix.size(), maxRows);
        size_t cols = (rows > 0) ? std::min(matrix[0].size(), maxCols) : 0;
        
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                std::cout << matrix[i][j] << "\t";
            }
            if (cols < matrix[0].size()) {
                std::cout << "...";
            }
            std::cout << std::endl;
        }
        if (rows < matrix.size()) {
            std::cout << "..." << std::endl;
        }
    }
};

int main() {
    const size_t SIZE = 1000;
    
    std::cout << "Generating random matrices of size " << SIZE << "x" << SIZE << "..." << std::endl;
    auto matrixA = ParallelMatrixMultiplier::generateRandomMatrix(SIZE, SIZE);
    auto matrixB = ParallelMatrixMultiplier::generateRandomMatrix(SIZE, SIZE);
    
    std::cout << "First few elements of Matrix A:" << std::endl;
    ParallelMatrixMultiplier::printMatrix(matrixA);
    
    std::cout << "\nFirst few elements of Matrix B:" << std::endl;
    ParallelMatrixMultiplier::printMatrix(matrixB);
    
    std::cout << "\nPerforming parallel matrix multiplication..." << std::endl;
    double startTime = omp_get_wtime();
    
    auto result = ParallelMatrixMultiplier::multiply(matrixA, matrixB);
    
    double endTime = omp_get_wtime();
    double executionTime = endTime - startTime;
    
    std::cout << "First few elements of Result Matrix:" << std::endl;
    ParallelMatrixMultiplier::printMatrix(result);
    
    std::cout << "\nExecution time: " << executionTime << " seconds" << std::endl;
    std::cout << "Performance: " << (2.0 * SIZE * SIZE * SIZE) / (executionTime * 1e9) << " GFLOPS" << std::endl;
    
    return 0;
}
#include <iostream>
#include <vector>
#include <chrono>
#include <omp.h>

std::vector<std::vector<double>> generate_matrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<double>(i + j) / 100.0;
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

bool verify_results(const std::vector<std::vector<double>>& mat1,
                    const std::vector<std::vector<double>>& mat2,
                    double tolerance = 1e-10) {
    if (mat1.size() != mat2.size() || mat1[0].size() != mat2[0].size()) {
        return false;
    }
    
    for (size_t i = 0; i < mat1.size(); ++i) {
        for (size_t j = 0; j < mat1[0].size(); ++j) {
            if (std::abs(mat1[i][j] - mat2[i][j]) > tolerance) {
                return false;
            }
        }
    }
    return true;
}

int main() {
    const int N = 500;
    
    auto matrix_A = generate_matrix(N, N);
    auto matrix_B = generate_matrix(N, N);
    
    auto start_seq = std::chrono::high_resolution_clock::now();
    auto result_seq = multiply_matrices_sequential(matrix_A, matrix_B);
    auto end_seq = std::chrono::high_resolution_clock::now();
    auto duration_seq = std::chrono::duration_cast<std::chrono::milliseconds>(end_seq - start_seq);
    
    auto start_par = std::chrono::high_resolution_clock::now();
    auto result_par = multiply_matrices_parallel(matrix_A, matrix_B);
    auto end_par = std::chrono::high_resolution_clock::now();
    auto duration_par = std::chrono::duration_cast<std::chrono::milliseconds>(end_par - start_par);
    
    bool verification_passed = verify_results(result_seq, result_par);
    
    std::cout << "Matrix size: " << N << "x" << N << std::endl;
    std::cout << "Sequential execution time: " << duration_seq.count() << " ms" << std::endl;
    std::cout << "Parallel execution time: " << duration_par.count() << " ms" << std::endl;
    std::cout << "Speedup factor: " << static_cast<double>(duration_seq.count()) / duration_par.count() << std::endl;
    std::cout << "Verification passed: " << (verification_passed ? "Yes" : "No") << std::endl;
    
    return 0;
}