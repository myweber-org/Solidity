
#include <iostream>
#include <vector>
#include <omp.h>
#include <chrono>
#include <cstdlib>

class MatrixMultiplier {
public:
    static std::vector<std::vector<double>> multiply(const std::vector<std::vector<double>>& A,
                                                     const std::vector<std::vector<double>>& B) {
        size_t n = A.size();
        size_t m = B[0].size();
        size_t p = B.size();
        
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
        std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
        
        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                matrix[i][j] = static_cast<double>(rand()) / RAND_MAX * 100.0;
            }
        }
        
        return matrix;
    }
    
    static void printMatrix(const std::vector<std::vector<double>>& matrix) {
        for (const auto& row : matrix) {
            for (double val : row) {
                std::cout << val << "\t";
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    const size_t SIZE = 500;
    
    auto start_gen = std::chrono::high_resolution_clock::now();
    auto A = MatrixMultiplier::generateRandomMatrix(SIZE, SIZE);
    auto B = MatrixMultiplier::generateRandomMatrix(SIZE, SIZE);
    auto end_gen = std::chrono::high_resolution_clock::now();
    
    auto start_mult = std::chrono::high_resolution_clock::now();
    auto C = MatrixMultiplier::multiply(A, B);
    auto end_mult = std::chrono::high_resolution_clock::now();
    
    auto gen_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_gen - start_gen);
    auto mult_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_mult - start_mult);
    
    std::cout << "Matrix generation time: " << gen_duration.count() << " ms" << std::endl;
    std::cout << "Matrix multiplication time: " << mult_duration.count() << " ms" << std::endl;
    std::cout << "Total execution time: " << (gen_duration + mult_duration).count() << " ms" << std::endl;
    
    return 0;
}