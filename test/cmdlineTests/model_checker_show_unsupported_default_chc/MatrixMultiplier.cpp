
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

        double tolerance = 1e-9;
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                if (std::abs(result[i][j] - sequentialResult[i][j]) > tolerance) {
                    std::cerr << "Verification failed at position (" << i << "," << j << ")" << std::endl;
                    return;
                }
            }
        }
        std::cout << "Matrix multiplication verified successfully." << std::endl;
    }

    void printMatrix(const std::vector<std::vector<double>>& matrix, size_t maxRows = 5, size_t maxCols = 5) {
        size_t printRows = std::min(matrix.size(), maxRows);
        size_t printCols = (matrix.empty()) ? 0 : std::min(matrix[0].size(), maxCols);
        
        for (size_t i = 0; i < printRows; ++i) {
            for (size_t j = 0; j < printCols; ++j) {
                std::cout << matrix[i][j] << "\t";
            }
            if (printCols < matrix[0].size()) std::cout << "...";
            std::cout << std::endl;
        }
        if (printRows < matrix.size()) std::cout << "...\n" << std::endl;
    }

    void benchmark() {
        clock_t start, end;
        
        std::cout << "Benchmarking sequential multiplication..." << std::endl;
        start = clock();
        multiplySequential();
        end = clock();
        double sequentialTime = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        std::cout << "Sequential time: " << sequentialTime << " seconds" << std::endl;

        std::cout << "\nBenchmarking parallel multiplication..." << std::endl;
        start = clock();
        multiplyParallel();
        end = clock();
        double parallelTime = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        std::cout << "Parallel time: " << parallelTime << " seconds" << std::endl;

        std::cout << "\nSpeedup factor: " << sequentialTime / parallelTime << std::endl;
    }
};

int main() {
    try {
        const size_t ROWS_A = 500;
        const size_t COLS_A = 500;
        const size_t ROWS_B = 500;
        const size_t COLS_B = 500;

        std::cout << "Initializing matrices of size " 
                  << ROWS_A << "x" << COLS_A << " and " 
                  << ROWS_B << "x" << COLS_B << "..." << std::endl;
        
        ParallelMatrixMultiplier multiplier(ROWS_A, COLS_A, ROWS_B, COLS_B);
        
        multiplier.benchmark();
        multiplier.verifyMultiplication();
        
        std::cout << "\nFirst 5x5 elements of result matrix:" << std::endl;
        multiplier.printMatrix(multiplier.result);
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}