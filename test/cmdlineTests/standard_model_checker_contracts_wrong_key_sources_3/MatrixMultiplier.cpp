
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

    void initializeRandomMatrix(std::vector<std::vector<double>>& matrix, size_t rows, size_t cols) {
        matrix.resize(rows, std::vector<double>(cols));
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                matrix[i][j] = static_cast<double>(rand()) / RAND_MAX * 100.0;
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

    void displayMatrix(const std::vector<std::vector<double>>& matrix, size_t maxRows = 5, size_t maxCols = 5) const {
        size_t displayRows = std::min(matrix.size(), maxRows);
        size_t displayCols = (matrix.empty()) ? 0 : std::min(matrix[0].size(), maxCols);
        
        std::cout << "Matrix preview (first " << displayRows << "x" << displayCols << " elements):\n";
        for (size_t i = 0; i < displayRows; ++i) {
            for (size_t j = 0; j < displayCols; ++j) {
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
        
        double startTime = omp_get_wtime();
        multiplySequential();
        double seqTime = omp_get_wtime() - startTime;
        
        std::cout << "Sequential multiplication time: " << seqTime << " seconds\n";
        seqResult = result;
        
        startTime = omp_get_wtime();
        multiplyParallel();
        double parTime = omp_get_wtime() - startTime;
        
        std::cout << "Parallel multiplication time: " << parTime << " seconds\n";
        std::cout << "Speedup: " << seqTime / parTime << "x\n";
        
        if (verifyResults(seqResult, result)) {
            std::cout << "Results verification: PASSED\n";
        } else {
            std::cout << "Results verification: FAILED\n";
        }
    }
};

int main() {
    const size_t ROWS_A = 500;
    const size_t COLS_A = 500;
    const size_t ROWS_B = 500;
    const size_t COLS_B = 500;
    
    try {
        ParallelMatrixMultiplier multiplier(ROWS_A, COLS_A, ROWS_B, COLS_B);
        
        std::cout << "Matrix dimensions: " << ROWS_A << "x" << COLS_A << " * " 
                  << ROWS_B << "x" << COLS_B << "\n";
        std::cout << "Total operations: " 
                  << ROWS_A * COLS_A * COLS_B << "\n";
        
        multiplier.benchmarkMultiplication();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}