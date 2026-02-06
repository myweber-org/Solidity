
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

    void displayResult(int maxRows = 5, int maxCols = 5) const {
        int displayRows = std::min(maxRows, rowsA);
        int displayCols = std::min(maxCols, colsB);
        
        std::cout << "First " << displayRows << "x" << displayCols << " elements of result:\n";
        for (int i = 0; i < displayRows; ++i) {
            for (int j = 0; j < displayCols; ++j) {
                std::cout << result[i][j] << "\t";
            }
            std::cout << "\n";
        }
    }

    double verifyResult(const std::vector<std::vector<double>>& reference) const {
        double error = 0.0;
        for (int i = 0; i < rowsA; ++i) {
            for (int j = 0; j < colsB; ++j) {
                error += std::abs(result[i][j] - reference[i][j]);
            }
        }
        return error;
    }
};

int main() {
    const int SIZE = 500;
    
    try {
        ParallelMatrixMultiplier multiplier(SIZE, SIZE, SIZE, SIZE);
        
        std::cout << "Matrix multiplication of size " << SIZE << "x" << SIZE << "\n";
        
        double startTime = omp_get_wtime();
        multiplier.multiplySequential();
        double sequentialTime = omp_get_wtime() - startTime;
        std::cout << "Sequential execution time: " << sequentialTime << " seconds\n";
        
        auto sequentialResult = multiplier;
        
        startTime = omp_get_wtime();
        multiplier.multiplyParallel();
        double parallelTime = omp_get_wtime() - startTime;
        std::cout << "Parallel execution time: " << parallelTime << " seconds\n";
        
        double speedup = sequentialTime / parallelTime;
        std::cout << "Speedup: " << speedup << "x\n";
        
        double error = sequentialResult.verifyResult(multiplier.result);
        std::cout << "Numerical error between sequential and parallel: " << error << "\n";
        
        if (SIZE <= 10) {
            multiplier.displayResult();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}