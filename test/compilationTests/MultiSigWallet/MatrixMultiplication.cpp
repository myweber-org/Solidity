
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

    void initializeRandomMatrix(std::vector<std::vector<double>>& matrix, int rows, int cols) {
        matrix.resize(rows, std::vector<double>(cols));
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                matrix[i][j] = static_cast<double>(rand()) / RAND_MAX * 100.0;
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
        initializeRandomMatrix(matrixA, rowsA, colsA);
        initializeRandomMatrix(matrixB, rowsB, colsB);
        result.resize(rowsA, std::vector<double>(colsB, 0.0));
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
        #pragma omp parallel for collapse(2) schedule(dynamic)
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

    void verifyResults(const std::vector<std::vector<double>>& reference) {
        const double epsilon = 1e-9;
        for (int i = 0; i < rowsA; ++i) {
            for (int j = 0; j < colsB; ++j) {
                if (std::abs(result[i][j] - reference[i][j]) > epsilon) {
                    std::cerr << "Verification failed at (" << i << "," << j << ")" << std::endl;
                    return;
                }
            }
        }
        std::cout << "Matrix multiplication verified successfully" << std::endl;
    }

    void printMatrix(const std::vector<std::vector<double>>& matrix, int maxRows = 5, int maxCols = 5) {
        int printRows = std::min(maxRows, static_cast<int>(matrix.size()));
        int printCols = (matrix.empty()) ? 0 : std::min(maxCols, static_cast<int>(matrix[0].size()));
        
        for (int i = 0; i < printRows; ++i) {
            for (int j = 0; j < printCols; ++j) {
                std::cout << matrix[i][j] << "\t";
            }
            if (printCols < matrix[0].size()) std::cout << "...";
            std::cout << std::endl;
        }
        if (printRows < matrix.size()) std::cout << "...\n";
    }

    void benchmark() {
        std::vector<std::vector<double>> sequentialResult = result;
        
        clock_t start = clock();
        multiplySequential();
        sequentialResult = result;
        clock_t seqTime = clock() - start;
        
        start = clock();
        multiplyParallel();
        clock_t parTime = clock() - start;
        
        verifyResults(sequentialResult);
        
        std::cout << "Sequential time: " << static_cast<double>(seqTime) / CLOCKS_PER_SEC << " seconds" << std::endl;
        std::cout << "Parallel time: " << static_cast<double>(parTime) / CLOCKS_PER_SEC << " seconds" << std::endl;
        std::cout << "Speedup: " << static_cast<double>(seqTime) / parTime << "x" << std::endl;
    }
};

int main() {
    const int SIZE = 512;
    
    try {
        ParallelMatrixMultiplier multiplier(SIZE, SIZE, SIZE, SIZE);
        
        std::cout << "Matrix A (first 5x5 elements):" << std::endl;
        multiplier.printMatrix(multiplier.matrixA);
        
        std::cout << "\nMatrix B (first 5x5 elements):" << std::endl;
        multiplier.printMatrix(multiplier.matrixB);
        
        std::cout << "\nBenchmarking matrix multiplication of " << SIZE << "x" << SIZE << " matrices..." << std::endl;
        multiplier.benchmark();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}