
#include <iostream>
#include <vector>
#include <omp.h>
#include <cstdlib>
#include <ctime>

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
    ParallelMatrixMultiplier(int rA, int cA, int rB, int cB) : rowsA(rA), colsA(cA), rowsB(rB), colsB(cB) {
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

    void verifyResults(const std::vector<std::vector<double>>& seqResult) {
        const double epsilon = 1e-9;
        for (int i = 0; i < rowsA; ++i) {
            for (int j = 0; j < colsB; ++j) {
                if (std::abs(result[i][j] - seqResult[i][j]) > epsilon) {
                    std::cerr << "Result mismatch at [" << i << "][" << j << "]" << std::endl;
                    return;
                }
            }
        }
        std::cout << "Results verified successfully" << std::endl;
    }

    void benchmarkMultiplication() {
        std::vector<std::vector<double>> seqResult = result;
        
        double startTime = omp_get_wtime();
        multiplySequential();
        double seqTime = omp_get_wtime() - startTime;
        
        startTime = omp_get_wtime();
        multiplyParallel();
        double parTime = omp_get_wtime() - startTime;
        
        verifyResults(seqResult);
        
        std::cout << "Sequential time: " << seqTime << " seconds" << std::endl;
        std::cout << "Parallel time: " << parTime << " seconds" << std::endl;
        std::cout << "Speedup: " << seqTime / parTime << "x" << std::endl;
    }

    void displayMatrix(const std::vector<std::vector<double>>& matrix, int maxRows = 5, int maxCols = 5) {
        int displayRows = std::min(maxRows, static_cast<int>(matrix.size()));
        int displayCols = (matrix.empty()) ? 0 : std::min(maxCols, static_cast<int>(matrix[0].size()));
        
        for (int i = 0; i < displayRows; ++i) {
            for (int j = 0; j < displayCols; ++j) {
                std::cout << matrix[i][j] << "\t";
            }
            if (displayCols < matrix[0].size()) std::cout << "...";
            std::cout << std::endl;
        }
        if (displayRows < matrix.size()) std::cout << "...\n" << std::endl;
    }
};

int main() {
    try {
        const int rowsA = 500;
        const int colsA = 500;
        const int rowsB = 500;
        const int colsB = 500;
        
        std::cout << "Initializing matrices of size " 
                  << rowsA << "x" << colsA << " and " 
                  << rowsB << "x" << colsB << std::endl;
        
        ParallelMatrixMultiplier multiplier(rowsA, colsA, rowsB, colsB);
        
        std::cout << "\nBenchmarking matrix multiplication..." << std::endl;
        multiplier.benchmarkMultiplication();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}