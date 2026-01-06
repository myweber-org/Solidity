
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

    void printResultDimensions() const {
        std::cout << "Result matrix dimensions: " << rowsA << " x " << colsB << std::endl;
    }

    double computeTrace() const {
        if (rowsA != colsB) {
            throw std::runtime_error("Trace only defined for square matrices");
        }
        
        double trace = 0.0;
        #pragma omp parallel for reduction(+:trace)
        for (size_t i = 0; i < rowsA; ++i) {
            trace += result[i][i];
        }
        return trace;
    }
};

int main() {
    const size_t N = 512;
    
    try {
        ParallelMatrixMultiplier multiplier(N, N, N, N);
        
        double startTime = omp_get_wtime();
        multiplier.multiplySequential();
        double sequentialTime = omp_get_wtime() - startTime;
        
        startTime = omp_get_wtime();
        multiplier.multiplyParallel();
        double parallelTime = omp_get_wtime() - startTime;
        
        multiplier.verifyMultiplication();
        multiplier.printResultDimensions();
        
        std::cout << "Sequential execution time: " << sequentialTime << " seconds" << std::endl;
        std::cout << "Parallel execution time: " << parallelTime << " seconds" << std::endl;
        std::cout << "Speedup factor: " << sequentialTime / parallelTime << std::endl;
        
        double trace = multiplier.computeTrace();
        std::cout << "Matrix trace: " << trace << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}