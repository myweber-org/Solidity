
#include <iostream>
#include <vector>

std::vector<std::vector<int>> multiplyMatrices(const std::vector<std::vector<int>>& A,
                                               const std::vector<std::vector<int>>& B) {
    int rowsA = A.size();
    int colsA = A[0].size();
    int rowsB = B.size();
    int colsB = B[0].size();

    if (colsA != rowsB) {
        throw std::invalid_argument("Matrix dimensions are incompatible for multiplication.");
    }

    std::vector<std::vector<int>> result(rowsA, std::vector<int>(colsB, 0));

    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            for (int k = 0; k < colsA; ++k) {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    return result;
}

void printMatrix(const std::vector<std::vector<int>>& matrix) {
    for (const auto& row : matrix) {
        for (int val : row) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    std::vector<std::vector<int>> matrixA = {{1, 2, 3}, {4, 5, 6}};
    std::vector<std::vector<int>> matrixB = {{7, 8}, {9, 10}, {11, 12}};

    try {
        std::vector<std::vector<int>> product = multiplyMatrices(matrixA, matrixB);
        std::cout << "Result of matrix multiplication:" << std::endl;
        printMatrix(product);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
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

    void multiplyParallel() {
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

    void displayMatrix(const std::vector<std::vector<double>>& mat, 
                       size_t maxRows = 5, size_t maxCols = 5) const {
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

    void benchmarkMultiplication(int numThreads) {
        omp_set_num_threads(numThreads);
        
        double startTime = omp_get_wtime();
        multiplyParallel();
        double endTime = omp_get_wtime();
        
        std::cout << "Threads: " << numThreads 
                  << ", Time: " << endTime - startTime << " seconds" << std::endl;
    }

    void verifyMultiplication() {
        bool correct = true;
        const double epsilon = 1e-6;
        
        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sequentialSum = 0.0;
                for (size_t k = 0; k < colsA; ++k) {
                    sequentialSum += matrixA[i][k] * matrixB[k][j];
                }
                if (std::abs(result[i][j] - sequentialSum) > epsilon) {
                    #pragma omp critical
                    {
                        correct = false;
                        std::cout << "Mismatch at [" << i << "][" << j << "]: "
                                  << result[i][j] << " vs " << sequentialSum << std::endl;
                    }
                }
            }
        }
        
        if (correct) {
            std::cout << "Matrix multiplication verified successfully." << std::endl;
        } else {
            std::cout << "Matrix multiplication verification failed." << std::endl;
        }
    }
};

int main() {
    const size_t SIZE = 500;
    
    try {
        ParallelMatrixMultiplier multiplier(SIZE, SIZE, SIZE, SIZE);
        
        std::cout << "Benchmarking parallel matrix multiplication (" 
                  << SIZE << "x" << SIZE << " matrices):" << std::endl;
        
        for (int threads : {1, 2, 4, 8}) {
            multiplier.benchmarkMultiplication(threads);
        }
        
        multiplier.verifyMultiplication();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}