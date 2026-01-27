
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
    int size;

public:
    ParallelMatrixMultiplier(int n) : size(n) {
        matrixA.resize(n, std::vector<double>(n));
        matrixB.resize(n, std::vector<double>(n));
        result.resize(n, std::vector<double>(n, 0.0));
        initializeMatrices();
    }

    void initializeMatrices() {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        #pragma omp parallel for collapse(2)
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                matrixA[i][j] = static_cast<double>(rand()) / RAND_MAX;
                matrixB[i][j] = static_cast<double>(rand()) / RAND_MAX;
            }
        }
    }

    void multiply() {
        #pragma omp parallel for collapse(2)
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                double sum = 0.0;
                #pragma omp simd reduction(+:sum)
                for (int k = 0; k < size; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                result[i][j] = sum;
            }
        }
    }

    void verifyMultiplication() {
        std::vector<std::vector<double>> sequentialResult(size, std::vector<double>(size, 0.0));
        
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                double sum = 0.0;
                for (int k = 0; k < size; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                sequentialResult[i][j] = sum;
            }
        }

        bool correct = true;
        const double tolerance = 1e-10;
        
        for (int i = 0; i < size && correct; ++i) {
            for (int j = 0; j < size && correct; ++j) {
                if (std::abs(result[i][j] - sequentialResult[i][j]) > tolerance) {
                    correct = false;
                }
            }
        }

        std::cout << "Verification: " << (correct ? "PASSED" : "FAILED") << std::endl;
    }

    void benchmark() {
        double startTime = omp_get_wtime();
        multiply();
        double endTime = omp_get_wtime();
        
        std::cout << "Matrix size: " << size << "x" << size << std::endl;
        std::cout << "Execution time: " << (endTime - startTime) << " seconds" << std::endl;
        std::cout << "Threads used: " << omp_get_max_threads() << std::endl;
    }
};

int main() {
    const int MATRIX_SIZE = 512;
    
    ParallelMatrixMultiplier multiplier(MATRIX_SIZE);
    
    std::cout << "Parallel Matrix Multiplication Benchmark" << std::endl;
    std::cout << "========================================" << std::endl;
    
    multiplier.benchmark();
    multiplier.verifyMultiplication();
    
    return 0;
}
#include <iostream>
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
            throw std::invalid_argument("Matrix dimensions mismatch for multiplication");
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
#include <iostream>
#include <vector>
#include <omp.h>
#include <chrono>
#include <cstdlib>

std::vector<std::vector<double>> generateRandomMatrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX * 10.0;
        }
    }
    return matrix;
}

std::vector<std::vector<double>> multiplyMatricesParallel(
    const std::vector<std::vector<double>>& A,
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

std::vector<std::vector<double>> multiplyMatricesSequential(
    const std::vector<std::vector<double>>& A,
    const std::vector<std::vector<double>>& B) {
    
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();
    
    std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));
    
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

int main() {
    const int SIZE = 500;
    srand(42);
    
    auto matrixA = generateRandomMatrix(SIZE, SIZE);
    auto matrixB = generateRandomMatrix(SIZE, SIZE);
    
    auto start = std::chrono::high_resolution_clock::now();
    auto resultSeq = multiplyMatricesSequential(matrixA, matrixB);
    auto end = std::chrono::high_resolution_clock::now();
    auto durationSeq = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    start = std::chrono::high_resolution_clock::now();
    auto resultPar = multiplyMatricesParallel(matrixA, matrixB);
    end = std::chrono::high_resolution_clock::now();
    auto durationPar = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Matrix size: " << SIZE << "x" << SIZE << std::endl;
    std::cout << "Sequential multiplication time: " << durationSeq.count() << " ms" << std::endl;
    std::cout << "Parallel multiplication time: " << durationPar.count() << " ms" << std::endl;
    std::cout << "Speedup factor: " << static_cast<double>(durationSeq.count()) / durationPar.count() << std::endl;
    
    bool resultsMatch = true;
    for (int i = 0; i < SIZE && resultsMatch; ++i) {
        for (int j = 0; j < SIZE && resultsMatch; ++j) {
            if (std::abs(resultSeq[i][j] - resultPar[i][j]) > 1e-6) {
                resultsMatch = false;
            }
        }
    }
    
    if (resultsMatch) {
        std::cout << "Results verification: PASSED" << std::endl;
    } else {
        std::cout << "Results verification: FAILED" << std::endl;
    }
    
    return 0;
}