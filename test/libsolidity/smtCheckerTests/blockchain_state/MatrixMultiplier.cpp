
#include <iostream>
#include <vector>
#include <random>
#include <omp.h>
#include <chrono>

class MatrixMultiplier {
private:
    std::vector<std::vector<double>> matrixA;
    std::vector<std::vector<double>> matrixB;
    std::vector<std::vector<double>> result;
    size_t rowsA, colsA, rowsB, colsB;

    void initializeMatrices() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 10.0);

        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsA; ++j) {
                matrixA[i][j] = dis(gen);
            }
        }

        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < rowsB; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                matrixB[i][j] = dis(gen);
            }
        }
    }

public:
    MatrixMultiplier(size_t rA, size_t cA, size_t rB, size_t cB) 
        : rowsA(rA), colsA(cA), rowsB(rB), colsB(cB) {
        
        if (colsA != rowsB) {
            throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
        }

        matrixA.resize(rowsA, std::vector<double>(colsA));
        matrixB.resize(rowsB, std::vector<double>(colsB));
        result.resize(rowsA, std::vector<double>(colsB));

        initializeMatrices();
    }

    void multiplySequential() {
        auto start = std::chrono::high_resolution_clock::now();
        
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                result[i][j] = sum;
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Sequential multiplication completed in " << duration.count() << " ms\n";
    }

    void multiplyParallel() {
        auto start = std::chrono::high_resolution_clock::now();
        
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
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Parallel multiplication completed in " << duration.count() << " ms\n";
    }

    void verifyResult(const std::vector<std::vector<double>>& reference) {
        const double epsilon = 1e-6;
        bool correct = true;
        
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                if (std::abs(result[i][j] - reference[i][j]) > epsilon) {
                    correct = false;
                    std::cout << "Mismatch at [" << i << "][" << j << "]: " 
                              << result[i][j] << " vs " << reference[i][j] << "\n";
                }
            }
        }
        
        if (correct) {
            std::cout << "Result verification passed\n";
        } else {
            std::cout << "Result verification failed\n";
        }
    }

    void displayMatrix(const std::vector<std::vector<double>>& mat, size_t maxRows = 3, size_t maxCols = 3) {
        size_t displayRows = std::min(mat.size(), maxRows);
        size_t displayCols = (mat.empty()) ? 0 : std::min(mat[0].size(), maxCols);
        
        std::cout << "Matrix preview (first " << displayRows << "x" << displayCols << " elements):\n";
        for (size_t i = 0; i < displayRows; ++i) {
            for (size_t j = 0; j < displayCols; ++j) {
                std::cout << mat[i][j] << "\t";
            }
            std::cout << "\n";
        }
    }
};

int main() {
    const size_t rowsA = 500;
    const size_t colsA = 500;
    const size_t rowsB = 500;
    const size_t colsB = 500;

    try {
        MatrixMultiplier multiplier(rowsA, colsA, rowsB, colsB);
        
        std::cout << "Matrix dimensions: " << rowsA << "x" << colsA 
                  << " * " << rowsB << "x" << colsB << "\n";
        
        multiplier.multiplySequential();
        auto sequentialResult = multiplier;
        
        multiplier.multiplyParallel();
        
        sequentialResult.verifyResult(multiplier.result);
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
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

    void benchmarkMultiplication() {
        double startTime, endTime;
        
        std::cout << "Benchmarking matrix multiplication (" << rowsA << "x" << colsA << " * " << rowsB << "x" << colsB << ")\n";
        
        startTime = omp_get_wtime();
        multiplySequential();
        endTime = omp_get_wtime();
        std::cout << "Sequential execution time: " << (endTime - startTime) * 1000 << " ms\n";
        
        startTime = omp_get_wtime();
        multiplyParallel();
        endTime = omp_get_wtime();
        std::cout << "Parallel execution time: " << (endTime - startTime) * 1000 << " ms\n";
        
        std::cout << "Speedup factor: " << (rowsA * colsA * colsB) / 1e6 << " million operations\n";
    }

    const std::vector<std::vector<double>>& getResult() const {
        return result;
    }
};

int main() {
    try {
        const size_t SIZE = 512;
        ParallelMatrixMultiplier multiplier(SIZE, SIZE, SIZE, SIZE);
        
        multiplier.benchmarkMultiplication();
        
        std::cout << "\nVerifying small portion of result:\n";
        multiplier.displayMatrix(multiplier.getResult());
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}