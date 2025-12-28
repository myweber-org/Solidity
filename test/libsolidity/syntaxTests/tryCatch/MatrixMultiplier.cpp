
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
        size_t rowsB = B.size();
        size_t colsB = B[0].size();

        if (colsA != rowsB) {
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

    static void initializeRandomMatrix(std::vector<std::vector<double>>& matrix) {
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
            std::cout << std::endl;
        }
    }
};

int main() {
    const size_t N = 512;
    
    std::vector<std::vector<double>> matrixA(N, std::vector<double>(N));
    std::vector<std::vector<double>> matrixB(N, std::vector<double>(N));
    
    ParallelMatrixMultiplier::initializeRandomMatrix(matrixA);
    ParallelMatrixMultiplier::initializeRandomMatrix(matrixB);
    
    double startTime = omp_get_wtime();
    
    try {
        std::vector<std::vector<double>> result = 
            ParallelMatrixMultiplier::multiply(matrixA, matrixB);
        
        double endTime = omp_get_wtime();
        
        std::cout << "Matrix multiplication completed successfully." << std::endl;
        std::cout << "Matrix size: " << N << "x" << N << std::endl;
        std::cout << "Execution time: " << (endTime - startTime) << " seconds" << std::endl;
        
        if (N <= 8) {
            std::cout << "\nFirst 8x8 block of result matrix:" << std::endl;
            for (size_t i = 0; i < std::min(size_t(8), N); ++i) {
                for (size_t j = 0; j < std::min(size_t(8), N); ++j) {
                    std::cout << result[i][j] << "\t";
                }
                std::cout << std::endl;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}