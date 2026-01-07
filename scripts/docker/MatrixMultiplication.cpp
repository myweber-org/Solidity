
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

void initializeMatrix(std::vector<std::vector<double>>& matrix, int size) {
    matrix.resize(size, std::vector<double>(size));
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
        }
    }
}

void multiplyMatrices(const std::vector<std::vector<double>>& A,
                      const std::vector<std::vector<double>>& B,
                      std::vector<std::vector<double>>& C,
                      int size) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            double sum = 0.0;
            for (int k = 0; k < size; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

int main() {
    const int MATRIX_SIZE = 500;
    srand(static_cast<unsigned>(time(nullptr)));
    
    std::vector<std::vector<double>> A, B, C;
    
    initializeMatrix(A, MATRIX_SIZE);
    initializeMatrix(B, MATRIX_SIZE);
    C.resize(MATRIX_SIZE, std::vector<double>(MATRIX_SIZE));
    
    double start_time = omp_get_wtime();
    multiplyMatrices(A, B, C, MATRIX_SIZE);
    double end_time = omp_get_wtime();
    
    std::cout << "Matrix multiplication completed for size " << MATRIX_SIZE << std::endl;
    std::cout << "Execution time: " << (end_time - start_time) << " seconds" << std::endl;
    
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
    int size;

    void initializeMatrices() {
        matrixA.resize(size, std::vector<double>(size));
        matrixB.resize(size, std::vector<double>(size));
        result.resize(size, std::vector<double>(size, 0.0));

        #pragma omp parallel for collapse(2)
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                matrixA[i][j] = static_cast<double>(rand()) / RAND_MAX;
                matrixB[i][j] = static_cast<double>(rand()) / RAND_MAX;
            }
        }
    }

public:
    ParallelMatrixMultiplier(int n) : size(n) {
        srand(static_cast<unsigned>(time(nullptr)));
        initializeMatrices();
    }

    void multiply() {
        #pragma omp parallel for collapse(2)
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                double sum = 0.0;
                #pragma omp simd reduction(+:sum)
                for (int k = 0; k < size; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                result[i][j] = sum;
            }
        }
    }

    void verifyMultiplication() {
        bool correct = true;
        const double epsilon = 1e-6;

        #pragma omp parallel for collapse(2) reduction(&&:correct)
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                double sequential_sum = 0.0;
                for (int k = 0; k < size; ++k) {
                    sequential_sum += matrixA[i][k] * matrixB[k][j];
                }
                if (std::abs(result[i][j] - sequential_sum) > epsilon) {
                    correct = false;
                }
            }
        }

        std::cout << "Verification: " << (correct ? "PASSED" : "FAILED") << std::endl;
    }

    void printPerformanceStats() {
        double start_time = omp_get_wtime();
        multiply();
        double end_time = omp_get_wtime();

        std::cout << "Matrix size: " << size << "x" << size << std::endl;
        std::cout << "Execution time: " << (end_time - start_time) << " seconds" << std::endl;
        std::cout << "Threads used: " << omp_get_max_threads() << std::endl;
    }
};

int main() {
    const int MATRIX_SIZE = 512;
    
    ParallelMatrixMultiplier multiplier(MATRIX_SIZE);
    
    multiplier.printPerformanceStats();
    multiplier.verifyMultiplication();
    
    return 0;
}