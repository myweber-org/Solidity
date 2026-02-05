
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
        #pragma omp parallel for collapse(2)
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

    void displayMatrix(const std::vector<std::vector<double>>& matrix, size_t maxRows = 5, size_t maxCols = 5) const {
        size_t displayRows = std::min(matrix.size(), maxRows);
        size_t displayCols = (matrix.empty()) ? 0 : std::min(matrix[0].size(), maxCols);
        
        std::cout << "Matrix preview (first " << displayRows << "x" << displayCols << "):\n";
        for (size_t i = 0; i < displayRows; ++i) {
            for (size_t j = 0; j < displayCols; ++j) {
                std::cout << matrix[i][j] << "\t";
            }
            std::cout << "\n";
        }
    }

    void benchmarkMultiplication() {
        std::cout << "Benchmarking matrix multiplication (" << rowsA << "x" << colsA << " * " << rowsB << "x" << colsB << ")\n";
        
        double startTime = omp_get_wtime();
        multiplySequential();
        double seqTime = omp_get_wtime() - startTime;
        std::cout << "Sequential execution time: " << seqTime << " seconds\n";
        
        startTime = omp_get_wtime();
        multiplyParallel();
        double parTime = omp_get_wtime() - startTime;
        std::cout << "Parallel execution time: " << parTime << " seconds\n";
        
        std::cout << "Speedup factor: " << seqTime / parTime << "\n";
    }

    bool verifyResult(const std::vector<std::vector<double>>& reference) const {
        if (result.size() != reference.size() || result[0].size() != reference[0].size()) {
            return false;
        }
        
        const double epsilon = 1e-9;
        for (size_t i = 0; i < result.size(); ++i) {
            for (size_t j = 0; j < result[0].size(); ++j) {
                if (std::abs(result[i][j] - reference[i][j]) > epsilon) {
                    return false;
                }
            }
        }
        return true;
    }
};

int main() {
    try {
        const size_t ROWS_A = 500;
        const size_t COLS_A = 500;
        const size_t ROWS_B = 500;
        const size_t COLS_B = 500;
        
        ParallelMatrixMultiplier multiplier(ROWS_A, COLS_A, ROWS_B, COLS_B);
        
        multiplier.benchmarkMultiplication();
        
        std::cout << "\nVerification test with smaller matrices:\n";
        ParallelMatrixMultiplier testMultiplier(3, 3, 3, 3);
        
        std::vector<std::vector<double>> sequentialResult(3, std::vector<double>(3));
        testMultiplier.multiplySequential();
        
        std::vector<std::vector<double>> parallelResult(3, std::vector<double>(3));
        testMultiplier.multiplyParallel();
        
        if (testMultiplier.verifyResult(sequentialResult)) {
            std::cout << "Verification passed: Sequential and parallel results match.\n";
        } else {
            std::cout << "Verification failed: Results differ.\n";
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
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
        for (size_t i = 0; i < rowsA && correct; ++i) {
            for (size_t j = 0; j < colsB && correct; ++j) {
                if (std::abs(result[i][j] - sequentialResult[i][j]) > epsilon) {
                    correct = false;
                }
            }
        }

        std::cout << "Verification: " << (correct ? "PASSED" : "FAILED") << std::endl;
    }

    void printPerformanceMetrics() {
        double startTime, endTime;
        
        startTime = omp_get_wtime();
        multiplySequential();
        endTime = omp_get_wtime();
        double sequentialTime = endTime - startTime;
        
        startTime = omp_get_wtime();
        multiplyParallel();
        endTime = omp_get_wtime();
        double parallelTime = endTime - startTime;
        
        std::cout << "Matrix dimensions: " << rowsA << "x" << colsA << " * " 
                  << rowsB << "x" << colsB << std::endl;
        std::cout << "Sequential time: " << sequentialTime << " seconds" << std::endl;
        std::cout << "Parallel time: " << parallelTime << " seconds" << std::endl;
        std::cout << "Speedup: " << sequentialTime / parallelTime << "x" << std::endl;
    }
};

int main() {
    try {
        const size_t SIZE = 500;
        ParallelMatrixMultiplier multiplier(SIZE, SIZE, SIZE, SIZE);
        
        multiplier.printPerformanceMetrics();
        multiplier.verifyMultiplication();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}