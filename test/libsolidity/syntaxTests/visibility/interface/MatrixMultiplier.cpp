
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
        result.resize(rowsA, std::vector<double>(colsB));
        
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
                result[i][j] = 0.0;
                for (size_t k = 0; k < colsA; ++k) {
                    result[i][j] += matrixA[i][k] * matrixB[k][j];
                }
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

    void displayMatrix(const std::vector<std::vector<double>>& mat, size_t maxRows = 5, size_t maxCols = 5) const {
        size_t displayRows = std::min(mat.size(), maxRows);
        size_t displayCols = (mat.empty()) ? 0 : std::min(mat[0].size(), maxCols);
        
        for (size_t i = 0; i < displayRows; ++i) {
            for (size_t j = 0; j < displayCols; ++j) {
                std::cout << mat[i][j] << "\t";
            }
            if (displayCols < mat[0].size()) {
                std::cout << "...";
            }
            std::cout << std::endl;
        }
        if (displayRows < mat.size()) {
            std::cout << "... (" << mat.size() - displayRows << " more rows)" << std::endl;
        }
    }

    void benchmarkMultiplication() {
        double startTime, endTime;
        
        std::cout << "Matrix A dimensions: " << rowsA << "x" << colsA << std::endl;
        std::cout << "Matrix B dimensions: " << rowsB << "x" << colsB << std::endl;
        
        startTime = omp_get_wtime();
        multiplySequential();
        endTime = omp_get_wtime();
        std::cout << "Sequential multiplication time: " << (endTime - startTime) << " seconds" << std::endl;
        
        startTime = omp_get_wtime();
        multiplyParallel();
        endTime = omp_get_wtime();
        std::cout << "Parallel multiplication time: " << (endTime - startTime) << " seconds" << std::endl;
        
        std::cout << "\nFirst 5x5 elements of result matrix:" << std::endl;
        displayMatrix(result);
    }

    const std::vector<std::vector<double>>& getResult() const {
        return result;
    }
};

int main() {
    try {
        const size_t rowsA = 500;
        const size_t colsA = 500;
        const size_t rowsB = 500;
        const size_t colsB = 500;
        
        ParallelMatrixMultiplier multiplier(rowsA, colsA, rowsB, colsB);
        multiplier.benchmarkMultiplication();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}