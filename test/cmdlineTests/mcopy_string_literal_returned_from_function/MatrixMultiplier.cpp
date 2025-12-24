
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

    void verifyResults(const std::vector<std::vector<double>>& reference) {
        const double epsilon = 1e-9;
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                if (std::abs(result[i][j] - reference[i][j]) > epsilon) {
                    std::cerr << "Result verification failed at [" << i << "][" << j << "]" << std::endl;
                    return;
                }
            }
        }
        std::cout << "Result verification successful" << std::endl;
    }

    void benchmarkMultiplication() {
        std::vector<std::vector<double>> sequentialResult = result;
        
        clock_t start = clock();
        multiplySequential();
        clock_t end = clock();
        double sequentialTime = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        sequentialResult = result;
        
        start = clock();
        multiplyParallel();
        end = clock();
        double parallelTime = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        
        std::cout << "Sequential time: " << sequentialTime << " seconds" << std::endl;
        std::cout << "Parallel time: " << parallelTime << " seconds" << std::endl;
        std::cout << "Speedup: " << sequentialTime / parallelTime << "x" << std::endl;
        
        verifyResults(sequentialResult);
    }

    void displayMatrix(const std::vector<std::vector<double>>& matrix, size_t maxRows = 5, size_t maxCols = 5) {
        size_t displayRows = std::min(matrix.size(), maxRows);
        size_t displayCols = (matrix.empty()) ? 0 : std::min(matrix[0].size(), maxCols);
        
        std::cout << "Matrix preview (first " << displayRows << "x" << displayCols << "):" << std::endl;
        for (size_t i = 0; i < displayRows; ++i) {
            for (size_t j = 0; j < displayCols; ++j) {
                std::cout << matrix[i][j] << "\t";
            }
            std::cout << std::endl;
        }
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
                  << rowsB << "x" << colsB << std::endl;
        
        ParallelMatrixMultiplier multiplier(rowsA, colsA, rowsB, colsB);
        multiplier.benchmarkMultiplication();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}