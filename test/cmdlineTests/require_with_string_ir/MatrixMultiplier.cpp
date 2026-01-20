
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

    void verifyMultiplication() {
        std::vector<std::vector<double>> sequentialResult(rowsA, std::vector<double>(colsB, 0.0));
        
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                sequentialResult[i][j] = sum;
            }
        }

        bool correct = true;
        const double epsilon = 1e-9;
        for (size_t i = 0; i < rowsA && correct; ++i) {
            for (size_t j = 0; j < colsB && correct; ++j) {
                if (std::abs(result[i][j] - sequentialResult[i][j]) > epsilon) {
                    correct = false;
                }
            }
        }

        std::cout << "Verification: " << (correct ? "PASSED" : "FAILED") << std::endl;
    }

    void benchmark() {
        double startTime, endTime;
        
        startTime = omp_get_wtime();
        multiplySequential();
        endTime = omp_get_wtime();
        std::cout << "Sequential execution time: " << (endTime - startTime) << " seconds" << std::endl;

        startTime = omp_get_wtime();
        multiplyParallel();
        endTime = omp_get_wtime();
        std::cout << "Parallel execution time: " << (endTime - startTime) << " seconds" << std::endl;
    }

    void displayMatrix(const std::vector<std::vector<double>>& matrix, size_t maxRows = 5, size_t maxCols = 5) {
        size_t displayRows = std::min(matrix.size(), maxRows);
        size_t displayCols = std::min(matrix[0].size(), maxCols);
        
        std::cout << "Matrix preview (first " << displayRows << "x" << displayCols << "):" << std::endl;
        for (size_t i = 0; i < displayRows; ++i) {
            for (size_t j = 0; j < displayCols; ++j) {
                std::cout << matrix[i][j] << "\t";
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    try {
        const size_t rowsA = 500;
        const size_t colsA = 500;
        const size_t rowsB = 500;
        const size_t colsB = 500;

        ParallelMatrixMultiplier multiplier(rowsA, colsA, rowsB, colsB);
        
        std::cout << "Matrix dimensions: " << rowsA << "x" << colsA << " * " << rowsB << "x" << colsB << std::endl;
        std::cout << "Total operations: " << rowsA * colsA * colsB << std::endl;
        
        multiplier.benchmark();
        multiplier.verifyMultiplication();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
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
    int rowsA, colsA, rowsB, colsB;

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

    void initializeMatrices() {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        
        #pragma omp parallel for collapse(2)
        for (int i = 0; i < rowsA; ++i) {
            for (int j = 0; j < colsA; ++j) {
                matrixA[i][j] = static_cast<double>(rand() % 100) / 10.0;
            }
        }

        #pragma omp parallel for collapse(2)
        for (int i = 0; i < rowsB; ++i) {
            for (int j = 0; j < colsB; ++j) {
                matrixB[i][j] = static_cast<double>(rand() % 100) / 10.0;
            }
        }
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

    void displayResult(int maxRows = 5, int maxCols = 5) {
        std::cout << "Result matrix (first " << maxRows << "x" << maxCols << " elements):\n";
        for (int i = 0; i < std::min(rowsA, maxRows); ++i) {
            for (int j = 0; j < std::min(colsB, maxCols); ++j) {
                std::cout << result[i][j] << "\t";
            }
            std::cout << "\n";
        }
    }

    double verifyMultiplication() {
        double totalSum = 0.0;
        #pragma omp parallel for reduction(+:totalSum) collapse(2)
        for (int i = 0; i < rowsA; ++i) {
            for (int j = 0; j < colsB; ++j) {
                totalSum += result[i][j];
            }
        }
        return totalSum;
    }
};

int main() {
    const int SIZE = 512;
    
    try {
        ParallelMatrixMultiplier multiplier(SIZE, SIZE, SIZE, SIZE);
        
        double startTime = omp_get_wtime();
        multiplier.multiply();
        double endTime = omp_get_wtime();
        
        std::cout << "Matrix multiplication completed for " << SIZE << "x" << SIZE << " matrices\n";
        std::cout << "Execution time: " << (endTime - startTime) << " seconds\n";
        
        double checksum = multiplier.verifyMultiplication();
        std::cout << "Verification checksum: " << checksum << "\n";
        
        if (SIZE <= 10) {
            multiplier.displayResult();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}