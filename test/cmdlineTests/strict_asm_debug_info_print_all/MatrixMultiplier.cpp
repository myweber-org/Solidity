
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

public:
    ParallelMatrixMultiplier(size_t rA, size_t cA, size_t rB, size_t cB) 
        : rowsA(rA), colsA(cA), rowsB(rB), colsB(cB) {
        
        if (colsA != rowsB) {
            throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
        }

        matrixA.resize(rowsA, std::vector<double>(colsA));
        matrixB.resize(rowsB, std::vector<double>(colsB));
        result.resize(rowsA, std::vector<double>(colsB, 0.0));

        initializeRandomMatrices();
    }

    void initializeRandomMatrices() {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsA; ++j) {
                matrixA[i][j] = static_cast<double>(std::rand()) / RAND_MAX * 100.0;
            }
        }

        for (size_t i = 0; i < rowsB; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                matrixB[i][j] = static_cast<double>(std::rand()) / RAND_MAX * 100.0;
            }
        }
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

    void multiplyParallelOptimized(int chunkSize = 64) {
        #pragma omp parallel for schedule(dynamic, chunkSize)
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sum = 0.0;
                #pragma omp simd reduction(+:sum)
                for (size_t k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                result[i][j] = sum;
            }
        }
    }

    void verifyResult(const std::vector<std::vector<double>>& reference) {
        if (result.size() != reference.size() || result[0].size() != reference[0].size()) {
            std::cout << "Dimension mismatch in verification" << std::endl;
            return;
        }

        double tolerance = 1e-10;
        bool correct = true;

        for (size_t i = 0; i < result.size(); ++i) {
            for (size_t j = 0; j < result[0].size(); ++j) {
                if (std::abs(result[i][j] - reference[i][j]) > tolerance) {
                    correct = false;
                    std::cout << "Mismatch at (" << i << "," << j << "): " 
                              << result[i][j] << " vs " << reference[i][j] << std::endl;
                }
            }
        }

        if (correct) {
            std::cout << "Result verification passed" << std::endl;
        } else {
            std::cout << "Result verification failed" << std::endl;
        }
    }

    void printMatrix(const std::vector<std::vector<double>>& mat, size_t maxRows = 5, size_t maxCols = 5) {
        size_t rowsToPrint = std::min(mat.size(), maxRows);
        size_t colsToPrint = std::min(mat[0].size(), maxCols);

        for (size_t i = 0; i < rowsToPrint; ++i) {
            for (size_t j = 0; j < colsToPrint; ++j) {
                std::cout << mat[i][j] << "\t";
            }
            if (colsToPrint < mat[0].size()) {
                std::cout << "...";
            }
            std::cout << std::endl;
        }
        if (rowsToPrint < mat.size()) {
            std::cout << "... (" << mat.size() - rowsToPrint << " more rows)" << std::endl;
        }
    }

    void benchmarkMultiplication() {
        std::vector<std::vector<double>> sequentialResult = result;

        double startTime = omp_get_wtime();
        multiplySequential();
        double sequentialTime = omp_get_wtime() - startTime;
        sequentialResult = result;

        startTime = omp_get_wtime();
        multiplyParallel();
        double parallelTime = omp_get_wtime() - startTime;

        startTime = omp_get_wtime();
        multiplyParallelOptimized();
        double optimizedTime = omp_get_wtime() - startTime;

        std::cout << "Sequential time: " << sequentialTime << " seconds" << std::endl;
        std::cout << "Parallel time: " << parallelTime << " seconds" << std::endl;
        std::cout << "Optimized parallel time: " << optimizedTime << " seconds" << std::endl;
        std::cout << "Speedup (parallel): " << sequentialTime / parallelTime << "x" << std::endl;
        std::cout << "Speedup (optimized): " << sequentialTime / optimizedTime << "x" << std::endl;

        verifyResult(sequentialResult);
    }

    const std::vector<std::vector<double>>& getResult() const {
        return result;
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
        
        std::cout << "\nBenchmarking matrix multiplication..." << std::endl;
        multiplier.benchmarkMultiplication();

        std::cout << "\nFirst 5x5 elements of result matrix:" << std::endl;
        multiplier.printMatrix(multiplier.getResult());

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}