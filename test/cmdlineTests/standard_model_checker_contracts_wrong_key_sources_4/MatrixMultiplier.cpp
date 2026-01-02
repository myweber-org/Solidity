
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
        
        std::vector<std::vector<double>> C(n, std::vector<double>(m, 0.0));
        
        #pragma omp parallel for collapse(2) schedule(dynamic)
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < m; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < p; ++k) {
                    sum += A[i][k] * B[k][j];
                }
                C[i][j] = sum;
            }
        }
        
        return C;
    }
    
    static std::vector<std::vector<double>> generateRandomMatrix(size_t rows, size_t cols) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 10.0);
        
        std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                matrix[i][j] = dis(gen);
            }
        }
        return matrix;
    }
    
    static void printMatrix(const std::vector<std::vector<double>>& matrix) {
        for (const auto& row : matrix) {
            for (double val : row) {
                std::cout << val << " ";
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    const size_t N = 500;
    const size_t M = 500;
    const size_t P = 500;
    
    try {
        auto A = ParallelMatrixMultiplier::generateRandomMatrix(N, M);
        auto B = ParallelMatrixMultiplier::generateRandomMatrix(M, P);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        auto C = ParallelMatrixMultiplier::multiply(A, B);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "Matrix multiplication completed successfully." << std::endl;
        std::cout << "Matrix dimensions: " << N << "x" << M << " * " << M << "x" << P << std::endl;
        std::cout << "Execution time: " << duration.count() << " ms" << std::endl;
        
        #ifdef _OPENMP
            std::cout << "OpenMP version: " << _OPENMP << std::endl;
            std::cout << "Max threads available: " << omp_get_max_threads() << std::endl;
        #endif
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}