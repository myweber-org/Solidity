
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
                    double a_ik = matrixA[i][k];
                    for (size_t j = 0; j < colsB; ++j) {
                        localResult[i][j] += a_ik * matrixB[k][j];
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

    void printMatrix(const std::vector<std::vector<double>>& mat, size_t maxRows = 5, size_t maxCols = 5) {
        size_t printRows = std::min(maxRows, mat.size());
        size_t printCols = (mat.size() > 0) ? std::min(maxCols, mat[0].size()) : 0;

        for (size_t i = 0; i < printRows; ++i) {
            for (size_t j = 0; j < printCols; ++j) {
                std::cout << mat[i][j] << " ";
            }
            if (printCols < mat[0].size()) std::cout << "...";
            std::cout << std::endl;
        }
        if (printRows < mat.size()) std::cout << "..." << std::endl;
    }

    const std::vector<std::vector<double>>& getResult() const {
        return result;
    }

    void clearResult() {
        for (auto& row : result) {
            std::fill(row.begin(), row.end(), 0.0);
        }
    }
};

void benchmarkMultiplication(ParallelMatrixMultiplier& multiplier) {
    std::vector<std::vector<double>> sequentialResult;
    
    double start = omp_get_wtime();
    multiplier.multiplySequential();
    double seqTime = omp_get_wtime() - start;
    sequentialResult = multiplier.getResult();
    
    multiplier.clearResult();
    
    start = omp_get_wtime();
    multiplier.multiplyParallel();
    double parTime = omp_get_wtime() - start;
    
    bool correct = multiplier.verifyResult(sequentialResult);
    
    multiplier.clearResult();
    
    start = omp_get_wtime();
    multiplier.multiplyParallelOptimized();
    double parOptTime = omp_get_wtime() - start;
    
    bool correctOpt = multiplier.verifyResult(sequentialResult);
    
    std::cout << "Sequential time: " << seqTime << " seconds" << std::endl;
    std::cout << "Parallel time: " << parTime << " seconds" << std::endl;
    std::cout << "Parallel optimized time: " << parOptTime << " seconds" << std::endl;
    std::cout << "Speedup (parallel): " << seqTime / parTime << "x" << std::endl;
    std::cout << "Speedup (optimized): " << seqTime / parOptTime << "x" << std::endl;
    std::cout << "Results correct: " << (correct ? "Yes" : "No") << std::endl;
    std::cout << "Optimized results correct: " << (correctOpt ? "Yes" : "No") << std::endl;
}

int main() {
    const size_t SIZE = 500;
    
    try {
        std::cout << "Initializing matrices of size " << SIZE << "x" << SIZE << std::endl;
        ParallelMatrixMultiplier multiplier(SIZE, SIZE, SIZE, SIZE);
        
        std::cout << "\nBenchmarking matrix multiplication..." << std::endl;
        benchmarkMultiplication(multiplier);
        
        std::cout << "\nFirst few elements of result matrix:" << std::endl;
        multiplier.printMatrix(multiplier.getResult());
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}#include <iostream>
#include <vector>
#include <stdexcept>

std::vector<std::vector<double>> multiplyMatrices(const std::vector<std::vector<double>>& matrixA, const std::vector<std::vector<double>>& matrixB) {
    if (matrixA.empty() || matrixB.empty()) {
        throw std::invalid_argument("Input matrices cannot be empty.");
    }
    
    size_t rowsA = matrixA.size();
    size_t colsA = matrixA[0].size();
    size_t rowsB = matrixB.size();
    size_t colsB = matrixB[0].size();

    for (const auto& row : matrixA) {
        if (row.size() != colsA) {
            throw std::invalid_argument("Matrix A has inconsistent row sizes.");
        }
    }
    for (const auto& row : matrixB) {
        if (row.size() != colsB) {
            throw std::invalid_argument("Matrix B has inconsistent row sizes.");
        }
    }

    if (colsA != rowsB) {
        throw std::invalid_argument("Matrix dimensions are incompatible for multiplication.");
    }

    std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));

    for (size_t i = 0; i < rowsA; ++i) {
        for (size_t j = 0; j < colsB; ++j) {
            for (size_t k = 0; k < colsA; ++k) {
                result[i][j] += matrixA[i][k] * matrixB[k][j];
            }
        }
    }

    return result;
}

void printMatrix(const std::vector<std::vector<double>>& matrix) {
    for (const auto& row : matrix) {
        for (double val : row) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    try {
        std::vector<std::vector<double>> A = {{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};
        std::vector<std::vector<double>> B = {{7.0, 8.0}, {9.0, 10.0}, {11.0, 12.0}};

        std::vector<std::vector<double>> C = multiplyMatrices(A, B);

        std::cout << "Resultant matrix:" << std::endl;
        printMatrix(C);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}