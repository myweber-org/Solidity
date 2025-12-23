
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
        
        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                matrix[i][j] = static_cast<double>(rand()) / RAND_MAX * 10.0;
            }
        }
        
        return matrix;
    }
};

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    const size_t N = 500;
    
    std::vector<std::vector<double>> A = ParallelMatrixMultiplier::generateRandomMatrix(N, N);
    std::vector<std::vector<double>> B = ParallelMatrixMultiplier::generateRandomMatrix(N, N);
    
    double start_time = omp_get_wtime();
    std::vector<std::vector<double>> C = ParallelMatrixMultiplier::multiply(A, B);
    double end_time = omp_get_wtime();
    
    std::cout << "Matrix multiplication completed for " << N << "x" << N << " matrices" << std::endl;
    std::cout << "Execution time: " << (end_time - start_time) << " seconds" << std::endl;
    
    return 0;
}