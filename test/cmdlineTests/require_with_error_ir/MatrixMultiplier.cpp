
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
        
        if (colsA != B.size()) {
            throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
        }
        
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
    
    static void fillRandom(std::vector<std::vector<double>>& matrix) {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        
        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < matrix.size(); ++i) {
            for (size_t j = 0; j < matrix[0].size(); ++j) {
                matrix[i][j] = static_cast<double>(std::rand()) / RAND_MAX * 100.0;
            }
        }
    }
    
    static void printMatrix(const std::vector<std::vector<double>>& matrix) {
        for (const auto& row : matrix) {
            for (double val : row) {
                std::cout << val << "\t";
            }
            std::cout << "\n";
        }
    }
};

int main() {
    const size_t N = 512;
    
    std::vector<std::vector<double>> A(N, std::vector<double>(N));
    std::vector<std::vector<double>> B(N, std::vector<double>(N));
    
    ParallelMatrixMultiplier::fillRandom(A);
    ParallelMatrixMultiplier::fillRandom(B);
    
    double start_time = omp_get_wtime();
    
    std::vector<std::vector<double>> C = ParallelMatrixMultiplier::multiply(A, B);
    
    double end_time = omp_get_wtime();
    
    std::cout << "Matrix multiplication completed for " << N << "x" << N << " matrices.\n";
    std::cout << "Execution time: " << (end_time - start_time) << " seconds\n";
    
    if (N <= 8) {
        std::cout << "\nFirst 8x8 block of result matrix:\n";
        std::vector<std::vector<double>> block(8, std::vector<double>(8));
        for (size_t i = 0; i < 8 && i < N; ++i) {
            for (size_t j = 0; j < 8 && j < N; ++j) {
                block[i][j] = C[i][j];
            }
        }
        ParallelMatrixMultiplier::printMatrix(block);
    }
    
    return 0;
}