
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

    void displayResult(size_t maxRows = 5, size_t maxCols = 5) const {
        size_t displayRows = std::min(maxRows, rowsA);
        size_t displayCols = std::min(maxCols, colsB);
        
        std::cout << "Result matrix (first " << displayRows << "x" << displayCols << " elements):\n";
        for (size_t i = 0; i < displayRows; ++i) {
            for (size_t j = 0; j < displayCols; ++j) {
                std::cout << result[i][j] << "\t";
            }
            std::cout << "\n";
        }
    }

    double verifyWithSequential() {
        std::vector<std::vector<double>> sequentialResult(rowsA, std::vector<double>(colsB, 0.0));
        
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                sequentialResult[i][j] = sum;
            }
        }

        double maxError = 0.0;
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double error = std::abs(result[i][j] - sequentialResult[i][j]);
                if (error > maxError) {
                    maxError = error;
                }
            }
        }
        
        return maxError;
    }
};

int main() {
    const size_t SIZE = 512;
    
    try {
        ParallelMatrixMultiplier multiplier(SIZE, SIZE, SIZE, SIZE);
        
        double startTime = omp_get_wtime();
        multiplier.multiplyParallel();
        double parallelTime = omp_get_wtime() - startTime;
        
        std::cout << "Parallel multiplication completed in " << parallelTime << " seconds\n";
        
        double verificationError = multiplier.verifyWithSequential();
        std::cout << "Maximum numerical error compared to sequential: " << verificationError << "\n";
        
        if (SIZE <= 10) {
            multiplier.displayResult();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
#include <iostream>
#include <vector>
#include <chrono>
#include <omp.h>

std::vector<std::vector<double>> generateRandomMatrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
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

std::vector<std::vector<double>> multiplyMatricesSequential(
    const std::vector<std::vector<double>>& A,
    const std::vector<std::vector<double>>& B) {
    
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();
    
    std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));
    
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

bool verifyResults(const std::vector<std::vector<double>>& mat1,
                   const std::vector<std::vector<double>>& mat2,
                   double tolerance = 1e-6) {
    
    if (mat1.size() != mat2.size() || mat1[0].size() != mat2[0].size()) {
        return false;
    }
    
    for (size_t i = 0; i < mat1.size(); ++i) {
        for (size_t j = 0; j < mat1[0].size(); ++j) {
            if (std::abs(mat1[i][j] - mat2[i][j]) > tolerance) {
                return false;
            }
        }
    }
    return true;
}

int main() {
    const int N = 500;
    const int M = 500;
    const int P = 500;
    
    srand(42);
    
    std::cout << "Generating random matrices of size " << N << "x" << M << " and " << M << "x" << P << std::endl;
    
    auto matrixA = generateRandomMatrix(N, M);
    auto matrixB = generateRandomMatrix(M, P);
    
    std::cout << "Performing sequential matrix multiplication..." << std::endl;
    auto startSeq = std::chrono::high_resolution_clock::now();
    auto resultSeq = multiplyMatricesSequential(matrixA, matrixB);
    auto endSeq = std::chrono::high_resolution_clock::now();
    auto durationSeq = std::chrono::duration_cast<std::chrono::milliseconds>(endSeq - startSeq);
    
    std::cout << "Performing parallel matrix multiplication..." << std::endl;
    auto startPar = std::chrono::high_resolution_clock::now();
    auto resultPar = multiplyMatricesParallel(matrixA, matrixB);
    auto endPar = std::chrono::high_resolution_clock::now();
    auto durationPar = std::chrono::duration_cast<std::chrono::milliseconds>(endPar - startPar);
    
    std::cout << "\nPerformance Results:" << std::endl;
    std::cout << "Sequential time: " << durationSeq.count() << " ms" << std::endl;
    std::cout << "Parallel time: " << durationPar.count() << " ms" << std::endl;
    std::cout << "Speedup: " << static_cast<double>(durationSeq.count()) / durationPar.count() << "x" << std::endl;
    
    std::cout << "\nVerifying results..." << std::endl;
    if (verifyResults(resultSeq, resultPar)) {
        std::cout << "Results match! Parallel implementation is correct." << std::endl;
    } else {
        std::cout << "ERROR: Results do not match!" << std::endl;
    }
    
    std::cout << "\nFirst few elements of result matrix:" << std::endl;
    for (int i = 0; i < std::min(3, N); ++i) {
        for (int j = 0; j < std::min(3, P); ++j) {
            std::cout << resultPar[i][j] << " ";
        }
        std::cout << std::endl;
    }
    
    return 0;
}