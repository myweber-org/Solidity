
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
        matrixA.resize(rowsA, std::vector<double>(colsA));
        matrixB.resize(rowsB, std::vector<double>(colsB));
        result.resize(rowsA, std::vector<double>(colsB, 0.0));

        #pragma omp parallel for collapse(2)
        for (int i = 0; i < rowsA; ++i) {
            for (int j = 0; j < colsA; ++j) {
                matrixA[i][j] = static_cast<double>(rand()) / RAND_MAX * 100.0;
            }
        }

        #pragma omp parallel for collapse(2)
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
        srand(static_cast<unsigned>(time(nullptr)));
        initializeMatrices();
    }

    void multiply() {
        #pragma omp parallel for collapse(2)
        for (int i = 0; i < rowsA; ++i) {
            for (int j = 0; j < colsB; ++j) {
                double sum = 0.0;
                #pragma omp simd reduction(+:sum)
                for (int k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                result[i][j] = sum;
            }
        }
    }

    void displayResult(int limit = 5) const {
        int displayRows = std::min(limit, rowsA);
        int displayCols = std::min(limit, colsB);
        
        std::cout << "First " << displayRows << "x" << displayCols << " of result matrix:\n";
        for (int i = 0; i < displayRows; ++i) {
            for (int j = 0; j < displayCols; ++j) {
                std::cout << result[i][j] << "\t";
            }
            std::cout << "\n";
        }
    }

    double verifyWithSequential() {
        std::vector<std::vector<double>> sequential(rowsA, std::vector<double>(colsB, 0.0));
        
        for (int i = 0; i < rowsA; ++i) {
            for (int j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (int k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                sequential[i][j] = sum;
            }
        }

        double maxError = 0.0;
        for (int i = 0; i < rowsA; ++i) {
            for (int j = 0; j < colsB; ++j) {
                maxError = std::max(maxError, std::abs(result[i][j] - sequential[i][j]));
            }
        }
        return maxError;
    }
};

int main() {
    try {
        const int SIZE = 512;
        ParallelMatrixMultiplier multiplier(SIZE, SIZE, SIZE, SIZE);
        
        double startTime = omp_get_wtime();
        multiplier.multiply();
        double endTime = omp_get_wtime();
        
        std::cout << "Parallel matrix multiplication completed for " 
                  << SIZE << "x" << SIZE << " matrices.\n";
        std::cout << "Execution time: " << (endTime - startTime) << " seconds\n";
        
        double verificationError = multiplier.verifyWithSequential();
        std::cout << "Maximum numerical error compared to sequential: " 
                  << verificationError << "\n";
        
        if (SIZE <= 10) {
            multiplier.displayResult();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}