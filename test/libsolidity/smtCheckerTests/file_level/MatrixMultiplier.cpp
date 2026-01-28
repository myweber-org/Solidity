
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <omp.h>

class ParallelMatrixMultiplier {
private:
    std::vector<std::vector<double>> matrixA;
    std::vector<std::vector<double>> matrixB;
    std::vector<std::vector<double>> result;
    size_t rowsA, colsA, rowsB, colsB;

    void initializeRandomMatrix(std::vector<std::vector<double>>& matrix, 
                                size_t rows, size_t cols) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 10.0);

        matrix.resize(rows, std::vector<double>(cols));
        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                matrix[i][j] = dis(gen);
            }
        }
    }

public:
    ParallelMatrixMultiplier(size_t rA, size_t cA, size_t rB, size_t cB) 
        : rowsA(rA), colsA(cA), rowsB(rB), colsB(cB) {
        if (colsA != rowsB) {
            throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
        }

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

    void verifyWithSequential() {
        std::vector<std::vector<double>> sequential(rowsA, std::vector<double>(colsB, 0.0));
        
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                sequential[i][j] = sum;
            }
        }

        double maxError = 0.0;
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                maxError = std::max(maxError, std::abs(result[i][j] - sequential[i][j]));
            }
        }

        std::cout << "Verification complete. Maximum error: " << maxError << std::endl;
    }

    void benchmark() {
        auto start = std::chrono::high_resolution_clock::now();
        multiply();
        auto end = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double> duration = end - start;
        std::cout << "Parallel multiplication time: " << duration.count() << " seconds" << std::endl;
    }

    void displayResult(size_t maxRows = 5, size_t maxCols = 5) {
        std::cout << "Result matrix (first " << maxRows << "x" << maxCols << " elements):" << std::endl;
        for (size_t i = 0; i < std::min(rowsA, maxRows); ++i) {
            for (size_t j = 0; j < std::min(colsB, maxCols); ++j) {
                std::cout << result[i][j] << "\t";
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    const size_t SIZE = 512;
    
    try {
        ParallelMatrixMultiplier multiplier(SIZE, SIZE, SIZE, SIZE);
        
        std::cout << "Matrix dimensions: " << SIZE << "x" << SIZE << std::endl;
        std::cout << "Number of threads: " << omp_get_max_threads() << std::endl;
        
        multiplier.benchmark();
        multiplier.verifyWithSequential();
        multiplier.displayResult();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
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
    int size;

public:
    ParallelMatrixMultiplier(int n) : size(n) {
        matrixA.resize(n, std::vector<double>(n));
        matrixB.resize(n, std::vector<double>(n));
        result.resize(n, std::vector<double>(n, 0.0));
        initializeMatrices();
    }

    void initializeMatrices() {
        srand(static_cast<unsigned>(time(nullptr)));
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
                for (int k = 0; k < size; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                result[i][j] = sum;
            }
        }
    }

    void displayResult(int limit = 5) {
        std::cout << "First " << limit << "x" << limit << " elements of result matrix:\n";
        for (int i = 0; i < std::min(limit, size); ++i) {
            for (int j = 0; j < std::min(limit, size); ++j) {
                std::cout << result[i][j] << " ";
            }
            std::cout << "\n";
        }
    }

    double verifyMultiplication() {
        double checksum = 0.0;
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                checksum += result[i][j];
            }
        }
        return checksum;
    }
};

int main() {
    const int MATRIX_SIZE = 500;
    
    std::cout << "Initializing parallel matrix multiplier with size " << MATRIX_SIZE << "x" << MATRIX_SIZE << "\n";
    
    double startTime = omp_get_wtime();
    ParallelMatrixMultiplier multiplier(MATRIX_SIZE);
    double initTime = omp_get_wtime() - startTime;
    
    std::cout << "Matrix initialization completed in " << initTime << " seconds\n";
    
    startTime = omp_get_wtime();
    multiplier.multiply();
    double multiplyTime = omp_get_wtime() - startTime;
    
    std::cout << "Matrix multiplication completed in " << multiplyTime << " seconds\n";
    std::cout << "Performance: " << (2.0 * MATRIX_SIZE * MATRIX_SIZE * MATRIX_SIZE) / (multiplyTime * 1e9) << " GFLOPS\n";
    
    multiplier.displayResult();
    
    double checksum = multiplier.verifyMultiplication();
    std::cout << "Result matrix checksum: " << checksum << "\n";
    
    return 0;
}