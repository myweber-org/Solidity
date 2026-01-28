
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

    void displayResult(size_t maxRows = 5, size_t maxCols = 5) const {
        size_t displayRows = std::min(maxRows, rowsA);
        size_t displayCols = std::min(maxCols, colsB);
        
        std::cout << "Result matrix (first " << displayRows << "x" << displayCols << " elements):\n";
        for (size_t i = 0; i < displayRows; ++i) {
            for (size_t j = 0; j < displayCols; ++j) {
                std::cout << result[i][j] << "\t";
            }
            std::cout << "\n";
        }
    }

    double verifyMultiplication() const {
        double totalSum = 0.0;
        for (const auto& row : result) {
            for (double val : row) {
                totalSum += val;
            }
        }
        return totalSum;
    }
};

int main() {
    const size_t SIZE = 500;
    
    try {
        ParallelMatrixMultiplier multiplier(SIZE, SIZE, SIZE, SIZE);
        
        double startTime = omp_get_wtime();
        multiplier.multiplySequential();
        double sequentialTime = omp_get_wtime() - startTime;
        
        std::cout << "Sequential multiplication time: " << sequentialTime << " seconds\n";
        
        startTime = omp_get_wtime();
        multiplier.multiplyParallel();
        double parallelTime = omp_get_wtime() - startTime;
        
        std::cout << "Parallel multiplication time: " << parallelTime << " seconds\n";
        std::cout << "Speedup factor: " << sequentialTime / parallelTime << "\n";
        
        double checksum = multiplier.verifyMultiplication();
        std::cout << "Matrix multiplication checksum: " << checksum << "\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}#include <iostream>
#include <vector>
#include <stdexcept>

std::vector<std::vector<double>> multiplyMatrices(const std::vector<std::vector<double>>& matrixA, const std::vector<std::vector<double>>& matrixB) {
    if (matrixA.empty() || matrixB.empty()) {
        throw std::invalid_argument("Input matrices cannot be empty.");
    }
    size_t rowsA = matrixA.size();
    size_t colsA = matrixA[0].size();
    size_t rowsB = matrixB.size();
    size_t colsB = matrixB[0].size();

    for (const auto& row : matrixA) {
        if (row.size() != colsA) {
            throw std::invalid_argument("Matrix A rows have inconsistent sizes.");
        }
    }
    for (const auto& row : matrixB) {
        if (row.size() != colsB) {
            throw std::invalid_argument("Matrix B rows have inconsistent sizes.");
        }
    }

    if (colsA != rowsB) {
        throw std::invalid_argument("Matrix A columns must equal Matrix B rows for multiplication.");
    }

    std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));

    for (size_t i = 0; i < rowsA; ++i) {
        for (size_t j = 0; j < colsB; ++j) {
            for (size_t k = 0; k < colsA; ++k) {
                result[i][j] += matrixA[i][k] * matrixB[k][j];
            }
        }
    }

    return result;
}

void printMatrix(const std::vector<std::vector<double>>& matrix) {
    for (const auto& row : matrix) {
        for (double val : row) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    try {
        std::vector<std::vector<double>> A = {{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};
        std::vector<std::vector<double>> B = {{7.0, 8.0}, {9.0, 10.0}, {11.0, 12.0}};

        std::vector<std::vector<double>> C = multiplyMatrices(A, B);

        std::cout << "Matrix A:" << std::endl;
        printMatrix(A);
        std::cout << "Matrix B:" << std::endl;
        printMatrix(B);
        std::cout << "Result of A * B:" << std::endl;
        printMatrix(C);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}