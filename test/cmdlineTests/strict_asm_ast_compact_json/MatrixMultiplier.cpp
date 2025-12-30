
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
    const size_t N = 500;
    
    std::cout << "Generating random matrices of size " << N << "x" << N << "..." << std::endl;
    auto matrixA = ParallelMatrixMultiplier::generateRandomMatrix(N, N);
    auto matrixB = ParallelMatrixMultiplier::generateRandomMatrix(N, N);
    
    std::cout << "Performing parallel matrix multiplication..." << std::endl;
    double start_time = omp_get_wtime();
    
    auto result = ParallelMatrixMultiplier::multiply(matrixA, matrixB);
    
    double end_time = omp_get_wtime();
    std::cout << "Multiplication completed in " << (end_time - start_time) << " seconds" << std::endl;
    
    std::cout << "Sample verification - first 3x3 block of result:" << std::endl;
    for (size_t i = 0; i < 3 && i < N; ++i) {
        for (size_t j = 0; j < 3 && j < N; ++j) {
            std::cout << result[i][j] << "\t";
        }
        std::cout << std::endl;
    }
    
    return 0;
}