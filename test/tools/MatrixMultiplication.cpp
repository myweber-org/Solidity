
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

    bool verifyResults(const std::vector<std::vector<double>>& seqResult) {
        const double epsilon = 1e-10;
        for (int i = 0; i < rowsA; ++i) {
            for (int j = 0; j < colsB; ++j) {
                if (std::abs(result[i][j] - seqResult[i][j]) > epsilon) {
                    return false;
                }
            }
        }
        return true;
    }

    void benchmarkMultiplication() {
        std::vector<std::vector<double>> sequentialResult = result;
        
        double startTime = omp_get_wtime();
        multiplySequential();
        double seqTime = omp_get_wtime() - startTime;
        sequentialResult = result;
        
        startTime = omp_get_wtime();
        multiplyParallel();
        double parTime = omp_get_wtime() - startTime;
        
        bool isValid = verifyResults(sequentialResult);
        
        std::cout << "Matrix dimensions: " << rowsA << "x" << colsA << " * " 
                  << rowsB << "x" << colsB << std::endl;
        std::cout << "Sequential time: " << seqTime << " seconds" << std::endl;
        std::cout << "Parallel time: " << parTime << " seconds" << std::endl;
        std::cout << "Speedup: " << seqTime / parTime << "x" << std::endl;
        std::cout << "Result validation: " << (isValid ? "PASSED" : "FAILED") << std::endl;
    }

    void displayMatrix(const std::vector<std::vector<double>>& matrix, int maxRows = 5, int maxCols = 5) {
        int displayRows = std::min(maxRows, static_cast<int>(matrix.size()));
        int displayCols = matrix.empty() ? 0 : std::min(maxCols, static_cast<int>(matrix[0].size()));
        
        for (int i = 0; i < displayRows; ++i) {
            for (int j = 0; j < displayCols; ++j) {
                std::cout << matrix[i][j] << "\t";
            }
            if (displayCols < matrix[0].size()) std::cout << "...";
            std::cout << std::endl;
        }
        if (displayRows < matrix.size()) std::cout << "...\n";
    }
};

int main() {
    try {
        const int SIZE = 512;
        ParallelMatrixMultiplier multiplier(SIZE, SIZE, SIZE, SIZE);
        
        std::cout << "Benchmarking matrix multiplication..." << std::endl;
        multiplier.benchmarkMultiplication();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

class ParallelMatrixMultiplier {
public:
    static std::vector<std::vector<double>> multiply(const std::vector<std::vector<double>>& A,
                                                     const std::vector<std::vector<double>>& B) {
        size_t n = A.size();
        size_t m = B[0].size();
        size_t p = B.size();

        if (A[0].size() != p) {
            throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
        }

        std::vector<std::vector<double>> C(n, std::vector<double>(m, 0.0));

        #pragma omp parallel for collapse(2) schedule(dynamic)
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < m; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < p; ++k) {
                    sum += A[i][k] * B[k][j];
                }
                C[i][j] = sum;
            }
        }

        return C;
    }

    static void initializeRandomMatrix(std::vector<std::vector<double>>& matrix) {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        for (auto& row : matrix) {
            for (auto& element : row) {
                element = static_cast<double>(std::rand()) / RAND_MAX * 100.0;
            }
        }
    }

    static void printMatrix(const std::vector<std::vector<double>>& matrix) {
        for (const auto& row : matrix) {
            for (const auto& element : row) {
                std::cout << element << "\t";
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    const size_t N = 4;
    const size_t M = 3;
    const size_t P = 5;

    std::vector<std::vector<double>> A(N, std::vector<double>(P));
    std::vector<std::vector<double>> B(P, std::vector<double>(M));
    std::vector<std::vector<double>> C;

    ParallelMatrixMultiplier::initializeRandomMatrix(A);
    ParallelMatrixMultiplier::initializeRandomMatrix(B);

    std::cout << "Matrix A (" << N << "x" << P << "):" << std::endl;
    ParallelMatrixMultiplier::printMatrix(A);
    std::cout << std::endl;

    std::cout << "Matrix B (" << P << "x" << M << "):" << std::endl;
    ParallelMatrixMultiplier::printMatrix(B);
    std::cout << std::endl;

    try {
        C = ParallelMatrixMultiplier::multiply(A, B);
        std::cout << "Result matrix C (" << N << "x" << M << "):" << std::endl;
        ParallelMatrixMultiplier::printMatrix(C);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}