
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
        
        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsA; ++j) {
                matrixA[i][j] = static_cast<double>(std::rand()) / RAND_MAX * 100.0;
            }
        }

        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < rowsB; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                matrixB[i][j] = static_cast<double>(std::rand()) / RAND_MAX * 100.0;
            }
        }
    }

    void multiplyMatrices() {
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

    void displayResult(size_t maxRows = 5, size_t maxCols = 5) const {
        std::cout << "Result matrix (first " << maxRows << "x" << maxCols << " elements):\n";
        for (size_t i = 0; i < std::min(rowsA, maxRows); ++i) {
            for (size_t j = 0; j < std::min(colsB, maxCols); ++j) {
                std::cout << result[i][j] << "\t";
            }
            std::cout << "\n";
        }
    }

    double verifyMultiplication() const {
        double totalSum = 0.0;
        #pragma omp parallel for reduction(+:totalSum) collapse(2)
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                totalSum += result[i][j];
            }
        }
        return totalSum;
    }
};

int main() {
    const size_t SIZE = 512;
    
    try {
        ParallelMatrixMultiplier multiplier(SIZE, SIZE, SIZE, SIZE);
        
        double startTime = omp_get_wtime();
        multiplier.multiplyMatrices();
        double endTime = omp_get_wtime();
        
        std::cout << "Matrix multiplication completed for " << SIZE << "x" << SIZE << " matrices\n";
        std::cout << "Execution time: " << (endTime - startTime) << " seconds\n";
        
        multiplier.displayResult();
        
        double verificationSum = multiplier.verifyMultiplication();
        std::cout << "Verification sum of all elements: " << verificationSum << "\n";
        
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

    void multiplyParallel(int numThreads = 4) {
        omp_set_num_threads(numThreads);
        
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

    void verifyResults(const std::vector<std::vector<double>>& reference) {
        const double epsilon = 1e-9;
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                if (std::abs(result[i][j] - reference[i][j]) > epsilon) {
                    std::cerr << "Result verification failed at [" << i << "][" << j << "]" << std::endl;
                    return;
                }
            }
        }
        std::cout << "Result verification successful" << std::endl;
    }

    void printMatrix(const std::vector<std::vector<double>>& matrix, size_t maxRows = 5, size_t maxCols = 5) {
        size_t printRows = std::min(maxRows, matrix.size());
        size_t printCols = (matrix.empty()) ? 0 : std::min(maxCols, matrix[0].size());
        
        for (size_t i = 0; i < printRows; ++i) {
            for (size_t j = 0; j < printCols; ++j) {
                std::cout << matrix[i][j] << "\t";
            }
            if (printCols < matrix[0].size()) std::cout << "...";
            std::cout << std::endl;
        }
        if (printRows < matrix.size()) std::cout << "..." << std::endl;
    }

    void benchmarkMultiplication() {
        std::vector<std::vector<double>> sequentialResult = result;
        
        clock_t start = clock();
        multiplySequential();
        clock_t seqEnd = clock();
        sequentialResult = result;
        
        result = std::vector<std::vector<double>>(rowsA, std::vector<double>(colsB, 0.0));
        
        clock_t parStart = clock();
        multiplyParallel();
        clock_t parEnd = clock();
        
        double seqTime = double(seqEnd - start) / CLOCKS_PER_SEC;
        double parTime = double(parEnd - parStart) / CLOCKS_PER_SEC;
        
        std::cout << "Sequential time: " << seqTime << " seconds" << std::endl;
        std::cout << "Parallel time: " << parTime << " seconds" << std::endl;
        std::cout << "Speedup: " << seqTime / parTime << "x" << std::endl;
        
        verifyResults(sequentialResult);
    }
};

int main() {
    try {
        const size_t N = 500;
        ParallelMatrixMultiplier multiplier(N, N, N, N);
        
        std::cout << "Matrix multiplication benchmark for " << N << "x" << N << " matrices" << std::endl;
        multiplier.benchmarkMultiplication();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}