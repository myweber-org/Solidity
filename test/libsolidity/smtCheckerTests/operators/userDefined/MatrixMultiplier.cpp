
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
    std::cout << "Performance: " << (2.0 * SIZE * SIZE * SIZE / (executionTime * 1e9)) << " GFLOPS" << std::endl;
    
    return 0;
}