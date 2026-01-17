
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
    
    static void benchmark(size_t size) {
        auto A = generateRandomMatrix(size, size);
        auto B = generateRandomMatrix(size, size);
        
        auto start = std::chrono::high_resolution_clock::now();
        auto C = multiply(A, B);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Matrix " << size << "x" << size << " multiplication completed in "
                  << duration.count() << " ms" << std::endl;
        
        // Verify correctness with a sample element
        double sample_sum = 0.0;
        for (size_t k = 0; k < size; ++k) {
            sample_sum += A[0][k] * B[k][0];
        }
        
        if (std::abs(C[0][0] - sample_sum) < 1e-9) {
            std::cout << "Verification passed: C[0][0] = " << C[0][0] << std::endl;
        } else {
            std::cout << "Verification failed: expected " << sample_sum 
                      << ", got " << C[0][0] << std::endl;
        }
    }
};

int main() {
    std::cout << "Parallel Matrix Multiplication Benchmark" << std::endl;
    std::cout << "Number of available threads: " << omp_get_max_threads() << std::endl;
    
    // Test different matrix sizes
    std::vector<size_t> sizes = {256, 512, 1024};
    
    for (size_t size : sizes) {
        ParallelMatrixMultiplier::benchmark(size);
    }
    
    return 0;
}