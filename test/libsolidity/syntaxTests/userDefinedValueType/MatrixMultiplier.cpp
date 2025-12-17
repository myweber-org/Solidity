
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
        const double tolerance = 1e-9;
        for (size_t i = 0; i < rowsA && correct; ++i) {
            for (size_t j = 0; j < colsB && correct; ++j) {
                if (std::abs(result[i][j] - sequentialResult[i][j]) > tolerance) {
                    correct = false;
                }
            }
        }

        std::cout << "Verification: " << (correct ? "PASSED" : "FAILED") << std::endl;
    }

    void printResult(size_t maxRows = 5, size_t maxCols = 5) {
        size_t printRows = std::min(maxRows, rowsA);
        size_t printCols = std::min(maxCols, colsB);
        
        std::cout << "First " << printRows << "x" << printCols << " elements of result matrix:" << std::endl;
        for (size_t i = 0; i < printRows; ++i) {
            for (size_t j = 0; j < printCols; ++j) {
                std::cout << result[i][j] << "\t";
            }
            std::cout << std::endl;
        }
    }

    double benchmarkMultiplication(bool parallel = true) {
        double startTime = omp_get_wtime();
        
        if (parallel) {
            multiplyParallel();
        } else {
            multiplySequential();
        }
        
        double endTime = omp_get_wtime();
        return endTime - startTime;
    }
};

int main() {
    const size_t ROWS_A = 500;
    const size_t COLS_A = 500;
    const size_t ROWS_B = 500;
    const size_t COLS_B = 500;

    try {
        ParallelMatrixMultiplier multiplier(ROWS_A, COLS_A, ROWS_B, COLS_B);
        
        std::cout << "Matrix dimensions: " << ROWS_A << "x" << COLS_A 
                  << " * " << ROWS_B << "x" << COLS_B << std::endl;
        
        double seqTime = multiplier.benchmarkMultiplication(false);
        std::cout << "Sequential multiplication time: " << seqTime << " seconds" << std::endl;
        
        double parTime = multiplier.benchmarkMultiplication(true);
        std::cout << "Parallel multiplication time: " << parTime << " seconds" << std::endl;
        
        std::cout << "Speedup: " << seqTime / parTime << "x" << std::endl;
        
        multiplier.verifyMultiplication();
        
        if (ROWS_A <= 10 && COLS_B <= 10) {
            multiplier.printResult();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}