
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

    void displayMatrix(const std::vector<std::vector<double>>& matrix, int maxRows = 5, int maxCols = 5) const {
        int displayRows = std::min(maxRows, static_cast<int>(matrix.size()));
        int displayCols = (matrix.empty()) ? 0 : std::min(maxCols, static_cast<int>(matrix[0].size()));
        
        std::cout << "Matrix preview (first " << displayRows << "x" << displayCols << " elements):\n";
        for (int i = 0; i < displayRows; ++i) {
            for (int j = 0; j < displayCols; ++j) {
                std::cout << matrix[i][j] << "\t";
            }
            std::cout << "\n";
        }
    }

    bool verifyResults(const std::vector<std::vector<double>>& seqResult, 
                      const std::vector<std::vector<double>>& parResult) const {
        const double epsilon = 1e-9;
        for (size_t i = 0; i < seqResult.size(); ++i) {
            for (size_t j = 0; j < seqResult[0].size(); ++j) {
                if (std::abs(seqResult[i][j] - parResult[i][j]) > epsilon) {
                    return false;
                }
            }
        }
        return true;
    }

    void benchmarkMultiplication() {
        std::vector<std::vector<double>> seqResult = result;
        std::vector<std::vector<double>> parResult = result;

        double startTime = omp_get_wtime();
        multiplySequential();
        double seqTime = omp_get_wtime() - startTime;
        seqResult = result;

        startTime = omp_get_wtime();
        multiplyParallel();
        double parTime = omp_get_wtime() - startTime;
        parResult = result;

        std::cout << "Sequential execution time: " << seqTime << " seconds\n";
        std::cout << "Parallel execution time: " << parTime << " seconds\n";
        std::cout << "Speedup factor: " << seqTime / parTime << "\n";

        if (verifyResults(seqResult, parResult)) {
            std::cout << "Results verification: PASSED\n";
        } else {
            std::cout << "Results verification: FAILED\n";
        }
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
                  << rowsB << "x" << colsB << "...\n";

        ParallelMatrixMultiplier multiplier(rowsA, colsA, rowsB, colsB);
        
        std::cout << "\nMatrix A:\n";
        multiplier.displayMatrix(multiplier.matrixA);
        
        std::cout << "\nMatrix B:\n";
        multiplier.displayMatrix(multiplier.matrixB);
        
        std::cout << "\nBenchmarking matrix multiplication...\n";
        multiplier.benchmarkMultiplication();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}