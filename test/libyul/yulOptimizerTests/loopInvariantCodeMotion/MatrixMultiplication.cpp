
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

    bool verifyResult(const std::vector<std::vector<double>>& reference) {
        if (reference.size() != rowsA || reference[0].size() != colsB) {
            return false;
        }
        
        const double epsilon = 1e-9;
        for (int i = 0; i < rowsA; ++i) {
            for (int j = 0; j < colsB; ++j) {
                if (std::abs(result[i][j] - reference[i][j]) > epsilon) {
                    return false;
                }
            }
        }
        return true;
    }

    void printMatrix(const std::vector<std::vector<double>>& matrix, int maxRows = 5, int maxCols = 5) {
        int printRows = std::min(maxRows, static_cast<int>(matrix.size()));
        int printCols = (matrix.size() > 0) ? std::min(maxCols, static_cast<int>(matrix[0].size())) : 0;
        
        for (int i = 0; i < printRows; ++i) {
            for (int j = 0; j < printCols; ++j) {
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
        std::vector<std::vector<double>> sequentialResult;
        
        double startTime = omp_get_wtime();
        multiplySequential();
        sequentialResult = result;
        double sequentialTime = omp_get_wtime() - startTime;
        
        startTime = omp_get_wtime();
        multiplyParallel();
        double parallelTime = omp_get_wtime() - startTime;
        
        bool verification = verifyResult(sequentialResult);
        
        std::cout << "Matrix A dimensions: " << rowsA << "x" << colsA << std::endl;
        std::cout << "Matrix B dimensions: " << rowsB << "x" << colsB << std::endl;
        std::cout << "Result dimensions: " << rowsA << "x" << colsB << std::endl;
        std::cout << "Sequential execution time: " << sequentialTime << " seconds" << std::endl;
        std::cout << "Parallel execution time: " << parallelTime << " seconds" << std::endl;
        std::cout << "Speedup: " << sequentialTime / parallelTime << "x" << std::endl;
        std::cout << "Verification against sequential result: " 
                  << (verification ? "PASSED" : "FAILED") << std::endl;
        
        std::cout << "\nFirst few elements of result matrix:" << std::endl;
        printMatrix(result);
    }
};

int main() {
    try {
        const int rowsA = 500;
        const int colsA = 500;
        const int rowsB = 500;
        const int colsB = 500;
        
        std::cout << "Initializing parallel matrix multiplier..." << std::endl;
        ParallelMatrixMultiplier multiplier(rowsA, colsA, rowsB, colsB);
        
        std::cout << "Running benchmark..." << std::endl;
        multiplier.benchmarkMultiplication();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}