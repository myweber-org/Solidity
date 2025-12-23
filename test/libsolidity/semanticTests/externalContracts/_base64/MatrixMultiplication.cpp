
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

    void initializeRandomMatrix(std::vector<std::vector<double>>& mat, size_t rows, size_t cols) {
        mat.resize(rows, std::vector<double>(cols));
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                mat[i][j] = static_cast<double>(rand()) / RAND_MAX * 100.0;
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

    void verifyResults(const std::vector<std::vector<double>>& seqResult) {
        const double epsilon = 1e-9;
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                if (std::abs(result[i][j] - seqResult[i][j]) > epsilon) {
                    std::cerr << "Result mismatch at (" << i << "," << j << ")\n";
                    return;
                }
            }
        }
        std::cout << "Results verified successfully\n";
    }

    void benchmark() {
        std::vector<std::vector<double>> seqResult = result;
        
        clock_t start = clock();
        multiplySequential();
        clock_t end = clock();
        double seqTime = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        seqResult = result;
        
        start = clock();
        multiplyParallel();
        end = clock();
        double parTime = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        
        verifyResults(seqResult);
        
        std::cout << "Sequential time: " << seqTime << " seconds\n";
        std::cout << "Parallel time: " << parTime << " seconds\n";
        std::cout << "Speedup: " << seqTime / parTime << "x\n";
    }
};

int main() {
    try {
        const size_t rows = 500;
        const size_t cols = 500;
        
        ParallelMatrixMultiplier multiplier(rows, cols, cols, rows);
        multiplier.benchmark();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}