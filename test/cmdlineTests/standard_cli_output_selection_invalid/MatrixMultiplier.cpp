
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

class ParallelMatrixMultiplier {
private:
    std::vector<std::vector<double>> matrixA;
    std::vector<std::vector<double>> matrixB;
    std::vector<std::vector<double>> resultMatrix;
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
        resultMatrix.resize(rowsA, std::vector<double>(colsB, 0.0));
    }

    void multiplySequential() {
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                resultMatrix[i][j] = sum;
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
                resultMatrix[i][j] = sum;
            }
        }
    }

    void multiplyParallelOptimized(int chunkSize = 64) {
        #pragma omp parallel for schedule(dynamic, chunkSize)
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sum = 0.0;
                #pragma omp simd reduction(+:sum)
                for (size_t k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                resultMatrix[i][j] = sum;
            }
        }
    }

    bool verifyResult(const std::vector<std::vector<double>>& reference) {
        if (reference.size() != rowsA || reference[0].size() != colsB) {
            return false;
        }
        
        const double epsilon = 1e-9;
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                if (std::abs(resultMatrix[i][j] - reference[i][j]) > epsilon) {
                    return false;
                }
            }
        }
        return true;
    }

    void printMatrix(const std::vector<std::vector<double>>& matrix, size_t maxRows = 5, size_t maxCols = 5) {
        size_t printRows = std::min(matrix.size(), maxRows);
        size_t printCols = (matrix.size() > 0) ? std::min(matrix[0].size(), maxCols) : 0;
        
        for (size_t i = 0; i < printRows; ++i) {
            for (size_t j = 0; j < printCols; ++j) {
                std::cout << matrix[i][j] << "\t";
            }
            if (printCols < matrix[0].size()) {
                std::cout << "...";
            }
            std::cout << std::endl;
        }
        if (printRows < matrix.size()) {
            std::cout << "..." << std::endl;
        }
    }

    void benchmarkMultiplication() {
        std::vector<std::vector<double>> sequentialResult = resultMatrix;
        
        double startTime = omp_get_wtime();
        multiplySequential();
        double sequentialTime = omp_get_wtime() - startTime;
        sequentialResult = resultMatrix;
        
        startTime = omp_get_wtime();
        multiplyParallel();
        double parallelTime = omp_get_wtime() - startTime;
        
        startTime = omp_get_wtime();
        multiplyParallelOptimized();
        double optimizedTime = omp_get_wtime() - startTime;
        
        bool parallelCorrect = verifyResult(sequentialResult);
        bool optimizedCorrect = verifyResult(sequentialResult);
        
        std::cout << "Benchmark Results:" << std::endl;
        std::cout << "Matrix dimensions: " << rowsA << "x" << colsA << " * " << rowsB << "x" << colsB << std::endl;
        std::cout << "Sequential time: " << sequentialTime << " seconds" << std::endl;
        std::cout << "Parallel time: " << parallelTime << " seconds" << std::endl;
        std::cout << "Optimized parallel time: " << optimizedTime << " seconds" << std::endl;
        std::cout << "Speedup (parallel): " << sequentialTime / parallelTime << "x" << std::endl;
        std::cout << "Speedup (optimized): " << sequentialTime / optimizedTime << "x" << std::endl;
        std::cout << "Parallel result correct: " << (parallelCorrect ? "Yes" : "No") << std::endl;
        std::cout << "Optimized result correct: " << (optimizedCorrect ? "Yes" : "No") << std::endl;
    }
};

int main() {
    try {
        const size_t rowsA = 512;
        const size_t colsA = 512;
        const size_t rowsB = 512;
        const size_t colsB = 512;
        
        std::cout << "Initializing matrices..." << std::endl;
        ParallelMatrixMultiplier multiplier(rowsA, colsA, rowsB, colsB);
        
        std::cout << "Running benchmark..." << std::endl;
        multiplier.benchmarkMultiplication();
        
        std::cout << "\nFirst 5x5 elements of result matrix:" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}