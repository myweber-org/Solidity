
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
        for (auto& row : matrix) {
            for (auto& element : row) {
                element = static_cast<double>(std::rand()) / RAND_MAX * 100.0;
            }
        }
    }
    
    static bool verifyMultiplication(const std::vector<std::vector<double>>& A,
                                     const std::vector<std::vector<double>>& B,
                                     const std::vector<std::vector<double>>& C) {
        size_t rowsA = A.size();
        size_t colsB = B[0].size();
        size_t colsA = A[0].size();
        
        const double epsilon = 1e-9;
        
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < colsA; ++k) {
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
    
    std::vector<std::vector<double>> matrixA(N, std::vector<double>(N));
    std::vector<std::vector<double>> matrixB(N, std::vector<double>(N));
    
    ParallelMatrixMultiplier::fillRandom(matrixA);
    ParallelMatrixMultiplier::fillRandom(matrixB);
    
    double startTime = omp_get_wtime();
    auto result = ParallelMatrixMultiplier::multiply(matrixA, matrixB);
    double endTime = omp_get_wtime();
    
    std::cout << "Matrix multiplication completed in " << (endTime - startTime) << " seconds\n";
    
    if (ParallelMatrixMultiplier::verifyMultiplication(matrixA, matrixB, result)) {
        std::cout << "Result verification: PASSED\n";
    } else {
        std::cout << "Result verification: FAILED\n";
    }
    
    return 0;
}