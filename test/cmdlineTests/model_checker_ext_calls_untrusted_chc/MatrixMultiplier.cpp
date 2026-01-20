
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

class ParallelMatrixMultiplier {
private:
    std::vector<std::vector<double>> matrixA;
    std::vector<std::vector<double>> matrixB;
    std::vector<std::vector<double>> result;
    size_t rowsA, colsA, rowsB, colsB;

    void initializeRandomMatrix(std::vector<std::vector<double>>& matrix, size_t rows, size_t cols) {
        matrix.resize(rows, std::vector<double>(cols));
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                matrix[i][j] = static_cast<double>(rand()) / RAND_MAX * 100.0;
            }
        }
    }

public:
    ParallelMatrixMultiplier(size_t rA, size_t cA, size_t rB, size_t cB) 
        : rowsA(rA), colsA(cA), rowsB(rB), colsB(cB) {
        if (colsA != rowsB) {
            throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
        }
        
        srand(static_cast<unsigned>(time(nullptr)));
        initializeRandomMatrix(matrixA, rowsA, colsA);
        initializeRandomMatrix(matrixB, rowsB, colsB);
        result.resize(rowsA, std::vector<double>(colsB, 0.0));
    }

    void multiplySequential() {
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                result[i][j] = sum;
            }
        }
    }

    void multiplyParallel() {
        #pragma omp parallel for collapse(2) schedule(dynamic)
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                result[i][j] = sum;
            }
        }
    }

    void verifyMultiplication() {
        std::vector<std::vector<double>> sequentialResult(rowsA, std::vector<double>(colsB, 0.0));
        
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                sequentialResult[i][j] = sum;
            }
        }

        bool correct = true;
        const double epsilon = 1e-9;
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                if (std::abs(result[i][j] - sequentialResult[i][j]) > epsilon) {
                    correct = false;
                    break;
                }
            }
            if (!correct) break;
        }

        std::cout << "Verification: " << (correct ? "PASSED" : "FAILED") << std::endl;
    }

    void printMatrix(const std::vector<std::vector<double>>& matrix, size_t maxRows = 5, size_t maxCols = 5) {
        size_t printRows = std::min(matrix.size(), maxRows);
        size_t printCols = std::min(matrix[0].size(), maxCols);
        
        std::cout << "Matrix preview (first " << printRows << "x" << printCols << "):" << std::endl;
        for (size_t i = 0; i < printRows; ++i) {
            for (size_t j = 0; j < printCols; ++j) {
                std::cout << matrix[i][j] << "\t";
            }
            std::cout << std::endl;
        }
    }

    void benchmark() {
        double start, end;
        
        std::cout << "Benchmarking sequential multiplication..." << std::endl;
        start = omp_get_wtime();
        multiplySequential();
        end = omp_get_wtime();
        std::cout << "Sequential time: " << (end - start) << " seconds" << std::endl;

        std::cout << "Benchmarking parallel multiplication..." << std::endl;
        start = omp_get_wtime();
        multiplyParallel();
        end = omp_get_wtime();
        std::cout << "Parallel time: " << (end - start) << " seconds" << std::endl;
        
        verifyMultiplication();
    }
};

int main() {
    try {
        const size_t rowsA = 500;
        const size_t colsA = 500;
        const size_t rowsB = 500;
        const size_t colsB = 500;

        std::cout << "Initializing matrices of size " 
                  << rowsA << "x" << colsA << " and " 
                  << rowsB << "x" << colsB << "..." << std::endl;
        
        ParallelMatrixMultiplier multiplier(rowsA, colsA, rowsB, colsB);
        
        std::cout << "Matrix A: ";
        multiplier.printMatrix(multiplier.matrixA);
        std::cout << std::endl;
        
        std::cout << "Matrix B: ";
        multiplier.printMatrix(multiplier.matrixB);
        std::cout << std::endl;
        
        multiplier.benchmark();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}