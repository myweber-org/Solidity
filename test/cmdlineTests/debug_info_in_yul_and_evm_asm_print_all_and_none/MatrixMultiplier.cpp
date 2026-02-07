
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

    void verifyResults(const std::vector<std::vector<double>>& reference) {
        const double epsilon = 1e-9;
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                if (std::abs(result[i][j] - reference[i][j]) > epsilon) {
                    std::cerr << "Result verification failed at [" << i << "][" << j << "]\n";
                    return;
                }
            }
        }
        std::cout << "Result verification passed\n";
    }

    void benchmarkMultiplication() {
        std::vector<std::vector<double>> sequentialResult = result;
        
        clock_t start = clock();
        multiplySequential();
        clock_t end = clock();
        double sequentialTime = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        sequentialResult = result;
        
        start = clock();
        multiplyParallel();
        end = clock();
        double parallelTime = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        
        std::cout << "Sequential time: " << sequentialTime << " seconds\n";
        std::cout << "Parallel time: " << parallelTime << " seconds\n";
        std::cout << "Speedup: " << sequentialTime / parallelTime << "x\n";
        
        verifyResults(sequentialResult);
    }

    void displayMatrix(const std::vector<std::vector<double>>& matrix, size_t maxRows = 5, size_t maxCols = 5) {
        size_t displayRows = std::min(maxRows, matrix.size());
        size_t displayCols = (matrix.empty()) ? 0 : std::min(maxCols, matrix[0].size());
        
        for (size_t i = 0; i < displayRows; ++i) {
            for (size_t j = 0; j < displayCols; ++j) {
                std::cout << matrix[i][j] << "\t";
            }
            if (displayCols < matrix[0].size()) std::cout << "...";
            std::cout << "\n";
        }
        if (displayRows < matrix.size()) std::cout << "...\n";
    }
};

int main() {
    try {
        const size_t rowsA = 500;
        const size_t colsA = 500;
        const size_t rowsB = 500;
        const size_t colsB = 500;
        
        std::cout << "Initializing matrices for multiplication (" 
                  << rowsA << "x" << colsA << ") * (" 
                  << rowsB << "x" << colsB << ")...\n";
        
        ParallelMatrixMultiplier multiplier(rowsA, colsA, rowsB, colsB);
        
        std::cout << "\nBenchmarking matrix multiplication...\n";
        multiplier.benchmarkMultiplication();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

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

void printMatrix(const std::vector<std::vector<double>>& matrix, int maxRows = 3, int maxCols = 3) {
    int rows = std::min(static_cast<int>(matrix.size()), maxRows);
    int cols = std::min(static_cast<int>(matrix[0].size()), maxCols);
    
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            std::cout << matrix[i][j] << " ";
        }
        std::cout << "...\n";
    }
    std::cout << "...\n";
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    const int N = 500;
    const int M = 500;
    const int P = 500;
    
    std::cout << "Generating matrices of size " << N << "x" << M << " and " << M << "x" << P << "...\n";
    
    auto matrixA = generateRandomMatrix(N, M);
    auto matrixB = generateRandomMatrix(M, P);
    
    std::cout << "First few elements of matrix A:\n";
    printMatrix(matrixA);
    
    std::cout << "\nFirst few elements of matrix B:\n";
    printMatrix(matrixB);
    
    std::cout << "\nPerforming parallel matrix multiplication...\n";
    double startTime = omp_get_wtime();
    
    auto result = multiplyMatricesParallel(matrixA, matrixB);
    
    double endTime = omp_get_wtime();
    double elapsedTime = endTime - startTime;
    
    std::cout << "\nFirst few elements of result matrix:\n";
    printMatrix(result);
    
    std::cout << "\nMatrix multiplication completed in " << elapsedTime << " seconds.\n";
    std::cout << "Performance: " << (2.0 * N * M * P) / (elapsedTime * 1e9) << " GFLOPS\n";
    
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
    int rowsA, colsA, rowsB, colsB;

    void initializeRandomMatrix(std::vector<std::vector<double>>& matrix, int rows, int cols) {
        matrix.resize(rows, std::vector<double>(cols));
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                matrix[i][j] = static_cast<double>(rand()) / RAND_MAX * 100.0;
            }
        }
    }

public:
    ParallelMatrixMultiplier(int rA, int cA, int rB, int cB) 
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
        for (int i = 0; i < rowsA; ++i) {
            for (int j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (int k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                result[i][j] = sum;
            }
        }
    }

    void multiplyParallel() {
        #pragma omp parallel for collapse(2) schedule(dynamic)
        for (int i = 0; i < rowsA; ++i) {
            for (int j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (int k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                result[i][j] = sum;
            }
        }
    }

    void verifyResults(const std::vector<std::vector<double>>& reference) {
        const double epsilon = 1e-9;
        for (int i = 0; i < rowsA; ++i) {
            for (int j = 0; j < colsB; ++j) {
                if (std::abs(result[i][j] - reference[i][j]) > epsilon) {
                    std::cerr << "Result verification failed at [" << i << "][" << j << "]" << std::endl;
                    return;
                }
            }
        }
        std::cout << "Result verification passed" << std::endl;
    }

    void benchmarkMultiplication() {
        std::vector<std::vector<double>> sequentialResult = result;
        
        clock_t start = clock();
        multiplySequential();
        clock_t end = clock();
        double sequentialTime = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        sequentialResult = result;
        
        start = clock();
        multiplyParallel();
        end = clock();
        double parallelTime = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        
        std::cout << "Sequential time: " << sequentialTime << " seconds" << std::endl;
        std::cout << "Parallel time: " << parallelTime << " seconds" << std::endl;
        std::cout << "Speedup: " << sequentialTime / parallelTime << "x" << std::endl;
        
        verifyResults(sequentialResult);
    }

    void displayMatrix(const std::vector<std::vector<double>>& matrix, int maxRows = 5, int maxCols = 5) {
        int displayRows = std::min(maxRows, static_cast<int>(matrix.size()));
        int displayCols = (matrix.empty()) ? 0 : std::min(maxCols, static_cast<int>(matrix[0].size()));
        
        for (int i = 0; i < displayRows; ++i) {
            for (int j = 0; j < displayCols; ++j) {
                std::cout << matrix[i][j] << "\t";
            }
            if (displayCols < matrix[0].size()) std::cout << "...";
            std::cout << std::endl;
        }
        if (displayRows < matrix.size()) std::cout << "...\n" << std::endl;
    }
};

int main() {
    try {
        const int rowsA = 500;
        const int colsA = 500;
        const int rowsB = 500;
        const int colsB = 500;
        
        std::cout << "Initializing matrices of size " 
                  << rowsA << "x" << colsA << " and " 
                  << rowsB << "x" << colsB << std::endl;
        
        ParallelMatrixMultiplier multiplier(rowsA, colsA, rowsB, colsB);
        
        std::cout << "\nBenchmarking matrix multiplication..." << std::endl;
        multiplier.benchmarkMultiplication();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}