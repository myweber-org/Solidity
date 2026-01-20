
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

class ParallelMatrixMultiplier {
public:
    static std::vector<std::vector<double>> multiply(const std::vector<std::vector<double>>& A,
                                                     const std::vector<std::vector<double>>& B) {
        int rowsA = A.size();
        int colsA = A[0].size();
        int colsB = B[0].size();
        
        std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));
        
        #pragma omp parallel for collapse(2)
        for (int i = 0; i < rowsA; ++i) {
            for (int j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (int k = 0; k < colsA; ++k) {
                    sum += A[i][k] * B[k][j];
                }
                result[i][j] = sum;
            }
        }
        
        return result;
    }
    
    static void initializeRandomMatrix(std::vector<std::vector<double>>& matrix) {
        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < matrix.size(); ++i) {
            for (size_t j = 0; j < matrix[0].size(); ++j) {
                matrix[i][j] = static_cast<double>(rand()) / RAND_MAX * 100.0;
            }
        }
    }
    
    static bool validateMultiplication(const std::vector<std::vector<double>>& A,
                                       const std::vector<std::vector<double>>& B,
                                       const std::vector<std::vector<double>>& C) {
        int rowsA = A.size();
        int colsA = A[0].size();
        int colsB = B[0].size();
        
        for (int i = 0; i < rowsA; ++i) {
            for (int j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (int k = 0; k < colsA; ++k) {
                    sum += A[i][k] * B[k][j];
                }
                if (std::abs(sum - C[i][j]) > 1e-9) {
                    return false;
                }
            }
        }
        return true;
    }
};

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    const int N = 512;
    std::vector<std::vector<double>> matrixA(N, std::vector<double>(N));
    std::vector<std::vector<double>> matrixB(N, std::vector<double>(N));
    
    ParallelMatrixMultiplier::initializeRandomMatrix(matrixA);
    ParallelMatrixMultiplier::initializeRandomMatrix(matrixB);
    
    double startTime = omp_get_wtime();
    std::vector<std::vector<double>> result = 
        ParallelMatrixMultiplier::multiply(matrixA, matrixB);
    double endTime = omp_get_wtime();
    
    bool isValid = ParallelMatrixMultiplier::validateMultiplication(matrixA, matrixB, result);
    
    std::cout << "Matrix multiplication completed in " << (endTime - startTime) 
              << " seconds" << std::endl;
    std::cout << "Result validation: " << (isValid ? "PASSED" : "FAILED") << std::endl;
    
    return 0;
}
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
        #pragma omp parallel for collapse(2)
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

    void multiply() {
        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sum = 0.0;
                #pragma omp simd reduction(+:sum)
                for (size_t k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                result[i][j] = sum;
            }
        }
    }

    void displayMatrix(const std::vector<std::vector<double>>& matrix, size_t maxRows = 5, size_t maxCols = 5) const {
        size_t displayRows = std::min(maxRows, matrix.size());
        size_t displayCols = (matrix.empty()) ? 0 : std::min(maxCols, matrix[0].size());
        
        std::cout << "Matrix preview (first " << displayRows << "x" << displayCols << " elements):\n";
        for (size_t i = 0; i < displayRows; ++i) {
            for (size_t j = 0; j < displayCols; ++j) {
                std::cout << matrix[i][j] << "\t";
            }
            std::cout << "\n";
        }
    }

    void verifyMultiplication() const {
        if (result.empty() || result[0].empty()) return;
        
        std::cout << "\nVerification sample:\n";
        size_t testRow = std::min(static_cast<size_t>(2), rowsA - 1);
        size_t testCol = std::min(static_cast<size_t>(2), colsB - 1);
        
        double manualSum = 0.0;
        for (size_t k = 0; k < colsA; ++k) {
            manualSum += matrixA[testRow][k] * matrixB[k][testCol];
        }
        
        std::cout << "Result[" << testRow << "][" << testCol << "] = " << result[testRow][testCol] << "\n";
        std::cout << "Manual calculation = " << manualSum << "\n";
        std::cout << "Difference = " << std::abs(result[testRow][testCol] - manualSum) << "\n";
    }

    void benchmarkMultiplication(int numThreads) {
        omp_set_num_threads(numThreads);
        
        double startTime = omp_get_wtime();
        multiply();
        double endTime = omp_get_wtime();
        
        std::cout << "\nBenchmark with " << numThreads << " threads:\n";
        std::cout << "Execution time: " << (endTime - startTime) * 1000.0 << " ms\n";
        std::cout << "Matrix dimensions: " << rowsA << "x" << colsA << " * " 
                  << rowsB << "x" << colsB << " = " << rowsA << "x" << colsB << "\n";
    }
};

int main() {
    try {
        const size_t ROWS_A = 512;
        const size_t COLS_A = 512;
        const size_t ROWS_B = 512;
        const size_t COLS_B = 512;
        
        ParallelMatrixMultiplier multiplier(ROWS_A, COLS_A, ROWS_B, COLS_B);
        
        std::cout << "Parallel Matrix Multiplication using OpenMP\n";
        std::cout << "===========================================\n";
        
        multiplier.benchmarkMultiplication(1);
        multiplier.benchmarkMultiplication(2);
        multiplier.benchmarkMultiplication(4);
        multiplier.benchmarkMultiplication(8);
        
        multiplier.verifyMultiplication();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}