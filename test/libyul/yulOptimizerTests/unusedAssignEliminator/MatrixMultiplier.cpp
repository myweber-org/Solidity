
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <omp.h>

class ParallelMatrixMultiplier {
public:
    static std::vector<std::vector<double>> multiply(const std::vector<std::vector<double>>& A,
                                                     const std::vector<std::vector<double>>& B) {
        size_t n = A.size();
        size_t m = B[0].size();
        size_t p = B.size();
        
        if (A[0].size() != p) {
            throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
        }
        
        std::vector<std::vector<double>> result(n, std::vector<double>(m, 0.0));
        
        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < m; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < p; ++k) {
                    sum += A[i][k] * B[k][j];
                }
                result[i][j] = sum;
            }
        }
        
        return result;
    }
    
    static std::vector<std::vector<double>> generateRandomMatrix(size_t rows, size_t cols) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dist(0.0, 10.0);
        
        std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
        
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                matrix[i][j] = dist(gen);
            }
        }
        
        return matrix;
    }
    
    static bool verifyMultiplication(const std::vector<std::vector<double>>& A,
                                     const std::vector<std::vector<double>>& B,
                                     const std::vector<std::vector<double>>& C) {
        size_t n = A.size();
        size_t m = B[0].size();
        size_t p = B.size();
        
        const double epsilon = 1e-6;
        
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < m; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < p; ++k) {
                    sum += A[i][k] * B[k][j];
                }
                if (std::abs(sum - C[i][j]) > epsilon) {
                    return false;
                }
            }
        }
        return true;
    }
};

int main() {
    const size_t N = 512;
    
    try {
        auto matrixA = ParallelMatrixMultiplier::generateRandomMatrix(N, N);
        auto matrixB = ParallelMatrixMultiplier::generateRandomMatrix(N, N);
        
        auto start = std::chrono::high_resolution_clock::now();
        auto result = ParallelMatrixMultiplier::multiply(matrixA, matrixB);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        bool isValid = ParallelMatrixMultiplier::verifyMultiplication(matrixA, matrixB, result);
        
        std::cout << "Matrix multiplication completed for " << N << "x" << N << " matrices" << std::endl;
        std::cout << "Execution time: " << duration.count() << " ms" << std::endl;
        std::cout << "Result verification: " << (isValid ? "PASSED" : "FAILED") << std::endl;
        
        if (isValid) {
            std::cout << "Sample element [0][0]: " << result[0][0] << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}