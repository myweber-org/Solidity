
#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <omp.h>

std::vector<std::vector<double>> generate_random_matrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
        }
    }
    return matrix;
}

std::vector<std::vector<double>> multiply_matrices_parallel(
    const std::vector<std::vector<double>>& A,
    const std::vector<std::vector<double>>& B) {
    
    int m = A.size();
    int n = A[0].size();
    int p = B[0].size();
    
    std::vector<std::vector<double>> C(m, std::vector<double>(p, 0.0));
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < p; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
    
    return C;
}

std::vector<std::vector<double>> multiply_matrices_sequential(
    const std::vector<std::vector<double>>& A,
    const std::vector<std::vector<double>>& B) {
    
    int m = A.size();
    int n = A[0].size();
    int p = B[0].size();
    
    std::vector<std::vector<double>> C(m, std::vector<double>(p, 0.0));
    
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < p; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
    
    return C;
}

int main() {
    const int SIZE = 500;
    srand(42);
    
    auto A = generate_random_matrix(SIZE, SIZE);
    auto B = generate_random_matrix(SIZE, SIZE);
    
    auto start = std::chrono::high_resolution_clock::now();
    auto C_seq = multiply_matrices_sequential(A, B);
    auto end = std::chrono::high_resolution_clock::now();
    auto seq_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    start = std::chrono::high_resolution_clock::now();
    auto C_par = multiply_matrices_parallel(A, B);
    end = std::chrono::high_resolution_clock::now();
    auto par_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Sequential multiplication time: " << seq_duration.count() << " ms\n";
    std::cout << "Parallel multiplication time: " << par_duration.count() << " ms\n";
    std::cout << "Speedup: " << static_cast<double>(seq_duration.count()) / par_duration.count() << "x\n";
    
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
    ParallelMatrixMultiplier(int rA, int cA, int rB, int cB) : rowsA(rA), colsA(cA), rowsB(rB), colsB(cB) {
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

    void displayResult(int maxRows = 5, int maxCols = 5) const {
        int displayRows = std::min(maxRows, rowsA);
        int displayCols = std::min(maxCols, colsB);
        std::cout << "First " << displayRows << "x" << displayCols << " elements of result matrix:\n";
        for (int i = 0; i < displayRows; ++i) {
            for (int j = 0; j < displayCols; ++j) {
                std::cout << result[i][j] << "\t";
            }
            std::cout << "\n";
        }
    }

    double verifyWithSequential() {
        std::vector<std::vector<double>> sequentialResult(rowsA, std::vector<double>(colsB, 0.0));
        for (int i = 0; i < rowsA; ++i) {
            for (int j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (int k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                sequentialResult[i][j] = sum;
            }
        }

        double maxError = 0.0;
        for (int i = 0; i < rowsA; ++i) {
            for (int j = 0; j < colsB; ++j) {
                maxError = std::max(maxError, std::abs(result[i][j] - sequentialResult[i][j]));
            }
        }
        return maxError;
    }
};

int main() {
    const int SIZE = 500;
    try {
        ParallelMatrixMultiplier multiplier(SIZE, SIZE, SIZE, SIZE);
        
        double startTime = omp_get_wtime();
        multiplier.multiplySequential();
        double sequentialTime = omp_get_wtime() - startTime;
        std::cout << "Sequential multiplication time: " << sequentialTime << " seconds\n";

        startTime = omp_get_wtime();
        multiplier.multiplyParallel();
        double parallelTime = omp_get_wtime() - startTime;
        std::cout << "Parallel multiplication time: " << parallelTime << " seconds\n";
        
        std::cout << "Speedup factor: " << sequentialTime / parallelTime << "\n";
        
        double verificationError = multiplier.verifyWithSequential();
        std::cout << "Maximum numerical error: " << verificationError << "\n";
        
        if (verificationError < 1e-10) {
            std::cout << "Parallel computation verified successfully.\n";
        }
        
        if (SIZE <= 10) {
            multiplier.displayResult();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}