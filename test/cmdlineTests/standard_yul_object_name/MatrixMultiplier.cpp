
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

    void multiplyParallelOptimized() {
        #pragma omp parallel
        {
            std::vector<std::vector<double>> localResult(rowsA, std::vector<double>(colsB, 0.0));
            
            #pragma omp for
            for (size_t i = 0; i < rowsA; ++i) {
                for (size_t k = 0; k < colsA; ++k) {
                    double a = matrixA[i][k];
                    for (size_t j = 0; j < colsB; ++j) {
                        localResult[i][j] += a * matrixB[k][j];
                    }
                }
            }

            #pragma omp critical
            {
                for (size_t i = 0; i < rowsA; ++i) {
                    for (size_t j = 0; j < colsB; ++j) {
                        result[i][j] += localResult[i][j];
                    }
                }
            }
        }
    }

    bool verifyResult(const std::vector<std::vector<double>>& reference) {
        if (reference.size() != rowsA || reference[0].size() != colsB) {
            return false;
        }

        const double epsilon = 1e-10;
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                if (std::abs(result[i][j] - reference[i][j]) > epsilon) {
                    return false;
                }
            }
        }
        return true;
    }

    void printResult(size_t maxRows = 5, size_t maxCols = 5) {
        size_t printRows = std::min(maxRows, rowsA);
        size_t printCols = std::min(maxCols, colsB);

        std::cout << "Result matrix (first " << printRows << "x" << printCols << "):\n";
        for (size_t i = 0; i < printRows; ++i) {
            for (size_t j = 0; j < printCols; ++j) {
                std::cout << result[i][j] << "\t";
            }
            std::cout << "\n";
        }
    }
};

int main() {
    const size_t SIZE = 500;
    
    try {
        ParallelMatrixMultiplier multiplier(SIZE, SIZE, SIZE, SIZE);
        
        std::cout << "Matrix multiplication of size " << SIZE << "x" << SIZE << "\n";
        
        double start = omp_get_wtime();
        multiplier.multiplySequential();
        double sequentialTime = omp_get_wtime() - start;
        std::cout << "Sequential time: " << sequentialTime << " seconds\n";
        
        start = omp_get_wtime();
        multiplier.multiplyParallel();
        double parallelTime = omp_get_wtime() - start;
        std::cout << "Parallel time: " << parallelTime << " seconds\n";
        
        std::cout << "Speedup: " << sequentialTime / parallelTime << "x\n";
        
        multiplier.printResult();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}