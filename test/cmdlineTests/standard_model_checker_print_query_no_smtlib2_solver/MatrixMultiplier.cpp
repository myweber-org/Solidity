
#include <iostream>
#include <vector>
#include <chrono>
#include <omp.h>
#include <cstdlib>

class ParallelMatrixMultiplier {
public:
    static std::vector<std::vector<double>> multiply(const std::vector<std::vector<double>>& A,
                                                     const std::vector<std::vector<double>>& B) {
        size_t rowsA = A.size();
        size_t colsA = A[0].size();
        size_t rowsB = B.size();
        size_t colsB = B[0].size();
        
        if (colsA != rowsB) {
            throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
        }
        
        std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));
        
        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < colsA; ++k) {
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
    
    static void benchmark(size_t size) {
        std::vector<std::vector<double>> A(size, std::vector<double>(size));
        std::vector<std::vector<double>> B(size, std::vector<double>(size));
        
        initializeRandomMatrix(A);
        initializeRandomMatrix(B);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        auto result = multiply(A, B);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "Matrix multiplication of size " << size << "x" << size 
                  << " completed in " << duration.count() << " ms" << std::endl;
        
        double checksum = 0.0;
        for (const auto& row : result) {
            for (double val : row) {
                checksum += val;
            }
        }
        std::cout << "Result checksum: " << checksum << std::endl;
    }
};

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    std::cout << "OpenMP Matrix Multiplication Benchmark" << std::endl;
    std::cout << "Available threads: " << omp_get_max_threads() << std::endl;
    
    size_t sizes[] = {256, 512, 1024};
    
    for (size_t size : sizes) {
        ParallelMatrixMultiplier::benchmark(size);
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
        size_t displayCols = (displayRows > 0) ? std::min(matrix[0].size(), maxCols) : 0;
        
        std::cout << "Matrix preview (first " << displayRows << "x" << displayCols << "):\n";
        for (size_t i = 0; i < displayRows; ++i) {
            for (size_t j = 0; j < displayCols; ++j) {
                std::cout << matrix[i][j] << "\t";
            }
            std::cout << "\n";
        }
    }

    bool verifyResults(const std::vector<std::vector<double>>& seqResult) const {
        if (result.size() != seqResult.size() || result[0].size() != seqResult[0].size()) {
            return false;
        }
        
        const double epsilon = 1e-9;
        for (size_t i = 0; i < result.size(); ++i) {
            for (size_t j = 0; j < result[0].size(); ++j) {
                if (std::abs(result[i][j] - seqResult[i][j]) > epsilon) {
                    return false;
                }
            }
        }
        return true;
    }

    void benchmarkMultiplication() {
        std::vector<std::vector<double>> sequentialResult = result;
        
        clock_t start = clock();
        multiplySequential();
        sequentialResult = result;
        clock_t seqEnd = clock();
        
        double seqTime = static_cast<double>(seqEnd - start) / CLOCKS_PER_SEC;
        
        start = clock();
        multiplyParallel();
        clock_t parEnd = clock();
        
        double parTime = static_cast<double>(parEnd - start) / CLOCKS_PER_SEC;
        
        bool correct = verifyResults(sequentialResult);
        
        std::cout << "\nPerformance Benchmark:\n";
        std::cout << "Sequential time: " << seqTime << " seconds\n";
        std::cout << "Parallel time: " << parTime << " seconds\n";
        std::cout << "Speedup: " << seqTime / parTime << "x\n";
        std::cout << "Results correct: " << (correct ? "Yes" : "No") << "\n";
    }
};

int main() {
    try {
        const size_t rowsA = 500;
        const size_t colsA = 500;
        const size_t rowsB = 500;
        const size_t colsB = 500;
        
        std::cout << "Initializing matrices of size " 
                  << rowsA << "x" << colsA << " and " 
                  << rowsB << "x" << colsB << "...\n";
        
        ParallelMatrixMultiplier multiplier(rowsA, colsA, rowsB, colsB);
        
        std::cout << "\nMatrix A: ";
        multiplier.displayMatrix(multiplier.matrixA);
        
        std::cout << "\nMatrix B: ";
        multiplier.displayMatrix(multiplier.matrixB);
        
        multiplier.benchmarkMultiplication();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}