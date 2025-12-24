#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

class MatrixMultiplier {
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
                matrixA[i][j] = static_cast<double>(rand()) / RAND_MAX;
            }
        }

        #pragma omp parallel for collapse(2)
        for (int i = 0; i < rowsB; ++i) {
            for (int j = 0; j < colsB; ++j) {
                matrixB[i][j] = static_cast<double>(rand()) / RAND_MAX;
            }
        }
    }

public:
    MatrixMultiplier(int rA, int cA, int rB, int cB) 
        : rowsA(rA), colsA(cA), rowsB(rB), colsB(cB) {
        if (colsA != rowsB) {
            throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
        }
        srand(static_cast<unsigned>(time(nullptr)));
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

    void displayResult(int limit = 5) const {
        int displayRows = std::min(limit, rowsA);
        int displayCols = std::min(limit, colsB);
        
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
        MatrixMultiplier mm(SIZE, SIZE, SIZE, SIZE);
        
        std::cout << "Matrix multiplication for " << SIZE << "x" << SIZE << " matrices\n";
        
        double start = omp_get_wtime();
        mm.multiplySequential();
        double seqTime = omp_get_wtime() - start;
        std::cout << "Sequential execution time: " << seqTime << " seconds\n";
        
        MatrixMultiplier mmParallel(SIZE, SIZE, SIZE, SIZE);
        start = omp_get_wtime();
        mmParallel.multiplyParallel();
        double parTime = omp_get_wtime() - start;
        std::cout << "Parallel execution time: " << parTime << " seconds\n";
        
        std::cout << "Speedup: " << seqTime / parTime << "x\n";
        
        if (SIZE <= 10) {
            mmParallel.displayResult();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}