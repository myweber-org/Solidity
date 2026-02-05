
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

public:
    ParallelMatrixMultiplier(size_t rA, size_t cA, size_t rB, size_t cB) 
        : rowsA(rA), colsA(cA), rowsB(rB), colsB(cB) {
        
        if (colsA != rowsB) {
            throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
        }

        matrixA.resize(rowsA, std::vector<double>(colsA));
        matrixB.resize(rowsB, std::vector<double>(colsB));
        result.resize(rowsA, std::vector<double>(colsB, 0.0));

        initializeRandomMatrices();
    }

    void initializeRandomMatrices() {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsA; ++j) {
                matrixA[i][j] = static_cast<double>(std::rand()) / RAND_MAX * 100.0;
            }
        }

        for (size_t i = 0; i < rowsB; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                matrixB[i][j] = static_cast<double>(std::rand()) / RAND_MAX * 100.0;
            }
        }
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

    void multiplyParallelOptimized(int blockSize = 32) {
        #pragma omp parallel for
        for (size_t i = 0; i < rowsA; i += blockSize) {
            for (size_t j = 0; j < colsB; j += blockSize) {
                for (size_t k = 0; k < colsA; k += blockSize) {
                    size_t iEnd = std::min(i + blockSize, rowsA);
                    size_t jEnd = std::min(j + blockSize, colsB);
                    size_t kEnd = std::min(k + blockSize, colsA);
                    
                    for (size_t ii = i; ii < iEnd; ++ii) {
                        for (size_t jj = j; jj < jEnd; ++jj) {
                            double sum = result[ii][jj];
                            for (size_t kk = k; kk < kEnd; ++kk) {
                                sum += matrixA[ii][kk] * matrixB[kk][jj];
                            }
                            result[ii][jj] = sum;
                        }
                    }
                }
            }
        }
    }

    double verifyResult(const std::vector<std::vector<double>>& reference) {
        if (reference.size() != rowsA || reference[0].size() != colsB) {
            return -1.0;
        }

        double maxError = 0.0;
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double error = std::abs(result[i][j] - reference[i][j]);
                if (error > maxError) {
                    maxError = error;
                }
            }
        }
        return maxError;
    }

    void printResult(size_t maxRows = 5, size_t maxCols = 5) {
        size_t printRows = std::min(maxRows, rowsA);
        size_t printCols = std::min(maxCols, colsB);
        
        std::cout << "First " << printRows << "x" << printCols << " elements of result matrix:\n";
        for (size_t i = 0; i < printRows; ++i) {
            for (size_t j = 0; j < printCols; ++j) {
                std::cout << result[i][j] << "\t";
            }
            std::cout << "\n";
        }
    }
};

int main() {
    const size_t SIZE = 512;
    
    try {
        ParallelMatrixMultiplier multiplier(SIZE, SIZE, SIZE, SIZE);
        
        std::cout << "Matrix multiplication for " << SIZE << "x" << SIZE << " matrices\n";
        
        double startTime = omp_get_wtime();
        multiplier.multiplySequential();
        double seqTime = omp_get_wtime() - startTime;
        std::cout << "Sequential execution time: " << seqTime << " seconds\n";
        
        auto sequentialResult = multiplier;
        
        startTime = omp_get_wtime();
        multiplier.multiplyParallel();
        double parTime = omp_get_wtime() - startTime;
        std::cout << "Parallel execution time: " << parTime << " seconds\n";
        std::cout << "Speedup: " << seqTime / parTime << "x\n";
        
        double error = multiplier.verifyResult(sequentialResult.result);
        std::cout << "Maximum numerical error compared to sequential: " << error << "\n";
        
        if (error < 1e-10) {
            std::cout << "Results are numerically identical within tolerance.\n";
        }
        
        multiplier.printResult();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
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
        
        std::vector<std::vector<double>> C = MatrixMultiplier::multiply(A, B);
        
        std::cout << "\nResult A * B:" << std::endl;
        MatrixMultiplier::printMatrix(C);
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}