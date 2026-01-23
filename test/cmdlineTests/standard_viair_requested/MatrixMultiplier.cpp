
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

        double tolerance = 1e-9;
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                if (std::abs(result[i][j] - sequentialResult[i][j]) > tolerance) {
                    std::cerr << "Verification failed at position (" << i << ", " << j << ")" << std::endl;
                    return;
                }
            }
        }
        std::cout << "Matrix multiplication verified successfully" << std::endl;
    }

    void benchmarkPerformance() {
        clock_t start, end;
        
        start = clock();
        multiplySequential();
        end = clock();
        double sequentialTime = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        
        start = clock();
        multiplyParallel();
        end = clock();
        double parallelTime = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        
        std::cout << "Sequential execution time: " << sequentialTime << " seconds" << std::endl;
        std::cout << "Parallel execution time: " << parallelTime << " seconds" << std::endl;
        std::cout << "Speedup factor: " << sequentialTime / parallelTime << std::endl;
    }

    void displayMatrix(const std::vector<std::vector<double>>& matrix, size_t maxRows = 5, size_t maxCols = 5) {
        size_t displayRows = std::min(matrix.size(), maxRows);
        size_t displayCols = (matrix.empty()) ? 0 : std::min(matrix[0].size(), maxCols);
        
        for (size_t i = 0; i < displayRows; ++i) {
            for (size_t j = 0; j < displayCols; ++j) {
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
        const size_t SIZE = 500;
        ParallelMatrixMultiplier multiplier(SIZE, SIZE, SIZE, SIZE);
        
        std::cout << "Matrix multiplication benchmark for " << SIZE << "x" << SIZE << " matrices" << std::endl;
        multiplier.benchmarkPerformance();
        multiplier.verifyMultiplication();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}#include <iostream>
#include <vector>
#include <stdexcept>

class MatrixMultiplier {
public:
    static std::vector<std::vector<double>> multiply(const std::vector<std::vector<double>>& A,
                                                     const std::vector<std::vector<double>>& B) {
        if (A.empty() || B.empty()) {
            throw std::invalid_argument("Input matrices cannot be empty");
        }

        size_t rowsA = A.size();
        size_t colsA = A[0].size();
        size_t rowsB = B.size();
        size_t colsB = B[0].size();

        for (const auto& row : A) {
            if (row.size() != colsA) {
                throw std::invalid_argument("Matrix A has inconsistent row sizes");
            }
        }

        for (const auto& row : B) {
            if (row.size() != colsB) {
                throw std::invalid_argument("Matrix B has inconsistent row sizes");
            }
        }

        if (colsA != rowsB) {
            throw std::invalid_argument("Matrix dimension mismatch for multiplication");
        }

        std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));

        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                for (size_t k = 0; k < colsA; ++k) {
                    result[i][j] += A[i][k] * B[k][j];
                }
            }
        }

        return result;
    }

    static void printMatrix(const std::vector<std::vector<double>>& matrix) {
        for (const auto& row : matrix) {
            for (double val : row) {
                std::cout << val << " ";
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    try {
        std::vector<std::vector<double>> A = {{1, 2, 3}, {4, 5, 6}};
        std::vector<std::vector<double>> B = {{7, 8}, {9, 10}, {11, 12}};

        std::cout << "Matrix A:" << std::endl;
        MatrixMultiplier::printMatrix(A);
        std::cout << "\nMatrix B:" << std::endl;
        MatrixMultiplier::printMatrix(B);

        auto result = MatrixMultiplier::multiply(A, B);

        std::cout << "\nResult of A * B:" << std::endl;
        MatrixMultiplier::printMatrix(result);

        std::vector<std::vector<double>> C = {{1, 2}, {3, 4}};
        std::vector<std::vector<double>> D = {{5, 6, 7}, {8, 9, 10}};

        std::cout << "\nMatrix C:" << std::endl;
        MatrixMultiplier::printMatrix(C);
        std::cout << "\nMatrix D:" << std::endl;
        MatrixMultiplier::printMatrix(D);

        auto result2 = MatrixMultiplier::multiply(C, D);

        std::cout << "\nResult of C * D:" << std::endl;
        MatrixMultiplier::printMatrix(result2);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}