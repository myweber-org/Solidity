
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
    int rowsA, colsA, rowsB, colsB;

    void initializeMatrices() {
        srand(static_cast<unsigned>(time(nullptr)));
        
        for (int i = 0; i < rowsA; ++i) {
            for (int j = 0; j < colsA; ++j) {
                matrixA[i][j] = static_cast<double>(rand()) / RAND_MAX * 100.0;
            }
        }
        
        for (int i = 0; i < rowsB; ++i) {
            for (int j = 0; j < colsB; ++j) {
                matrixB[i][j] = static_cast<double>(rand()) / RAND_MAX * 100.0;
            }
        }
    }

public:
    ParallelMatrixMultiplier(int rA, int cA, int rB, int cB) 
        : rowsA(rA), colsA(cA), rowsB(rB), colsB(cB) {
        
        if (colsA != rowsB) {
            throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
        }
        
        matrixA.resize(rowsA, std::vector<double>(colsA));
        matrixB.resize(rowsB, std::vector<double>(colsB));
        result.resize(rowsA, std::vector<double>(colsB, 0.0));
        
        initializeMatrices();
    }

    void multiplySequential() {
        for (int i = 0; i < rowsA; ++i) {
            for (int j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (int k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                result[i][j] = sum;
            }
        }
    }

    void multiplyParallel() {
        #pragma omp parallel for collapse(2)
        for (int i = 0; i < rowsA; ++i) {
            for (int j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (int k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                result[i][j] = sum;
            }
        }
    }

    void multiplyParallelWithSchedule(int chunkSize) {
        #pragma omp parallel for collapse(2) schedule(dynamic, chunkSize)
        for (int i = 0; i < rowsA; ++i) {
            for (int j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (int k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                result[i][j] = sum;
            }
        }
    }

    void verifyResult(const std::vector<std::vector<double>>& reference) {
        const double epsilon = 1e-6;
        for (int i = 0; i < rowsA; ++i) {
            for (int j = 0; j < colsB; ++j) {
                if (std::abs(result[i][j] - reference[i][j]) > epsilon) {
                    std::cerr << "Result verification failed at [" << i << "][" << j << "]" << std::endl;
                    return;
                }
            }
        }
        std::cout << "Result verification successful" << std::endl;
    }

    void printMatrix(const std::vector<std::vector<double>>& matrix, int maxRows = 5, int maxCols = 5) {
        int rowsToPrint = std::min(maxRows, static_cast<int>(matrix.size()));
        int colsToPrint = maxCols;
        
        for (int i = 0; i < rowsToPrint; ++i) {
            if (i < matrix.size()) {
                colsToPrint = std::min(maxCols, static_cast<int>(matrix[i].size()));
                for (int j = 0; j < colsToPrint; ++j) {
                    std::cout << matrix[i][j] << "\t";
                }
                if (matrix[i].size() > maxCols) {
                    std::cout << "...";
                }
            }
            std::cout << std::endl;
        }
        if (matrix.size() > maxRows) {
            std::cout << "..." << std::endl;
        }
    }

    void benchmark() {
        std::vector<std::vector<double>> sequentialResult(rowsA, std::vector<double>(colsB, 0.0));
        
        double startTime = omp_get_wtime();
        multiplySequential();
        double sequentialTime = omp_get_wtime() - startTime;
        sequentialResult = result;
        
        std::fill(result.begin(), result.end(), std::vector<double>(colsB, 0.0));
        
        startTime = omp_get_wtime();
        multiplyParallel();
        double parallelTime = omp_get_wtime() - startTime;
        
        verifyResult(sequentialResult);
        
        std::cout << "Sequential execution time: " << sequentialTime << " seconds" << std::endl;
        std::cout << "Parallel execution time: " << parallelTime << " seconds" << std::endl;
        std::cout << "Speedup: " << sequentialTime / parallelTime << "x" << std::endl;
    }
};

int main() {
    try {
        const int rowsA = 500;
        const int colsA = 500;
        const int rowsB = 500;
        const int colsB = 500;
        
        std::cout << "Initializing matrices of size " << rowsA << "x" << colsA 
                  << " and " << rowsB << "x" << colsB << std::endl;
        
        ParallelMatrixMultiplier multiplier(rowsA, colsA, rowsB, colsB);
        
        std::cout << "\nRunning benchmark..." << std::endl;
        multiplier.benchmark();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}