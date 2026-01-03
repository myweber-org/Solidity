
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