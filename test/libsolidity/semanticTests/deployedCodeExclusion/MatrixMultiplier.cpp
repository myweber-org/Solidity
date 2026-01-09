
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

public:
    ParallelMatrixMultiplier(int n) : size(n) {
        matrixA.resize(n, std::vector<double>(n));
        matrixB.resize(n, std::vector<double>(n));
        result.resize(n, std::vector<double>(n, 0.0));
        initializeMatrices();
    }

    void initializeMatrices() {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                matrixA[i][j] = static_cast<double>(rand()) / RAND_MAX;
                matrixB[i][j] = static_cast<double>(rand()) / RAND_MAX;
            }
        }
    }

    void multiplySequential() {
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                double sum = 0.0;
                for (int k = 0; k < size; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                result[i][j] = sum;
            }
        }
    }

    void multiplyParallel() {
        #pragma omp parallel for collapse(2)
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                double sum = 0.0;
                for (int k = 0; k < size; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                result[i][j] = sum;
            }
        }
    }

    void verifyMultiplication() {
        std::vector<std::vector<double>> sequentialResult(size, std::vector<double>(size, 0.0));
        
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                double sum = 0.0;
                for (int k = 0; k < size; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                sequentialResult[i][j] = sum;
            }
        }

        bool correct = true;
        const double epsilon = 1e-10;
        for (int i = 0; i < size && correct; ++i) {
            for (int j = 0; j < size && correct; ++j) {
                if (std::abs(result[i][j] - sequentialResult[i][j]) > epsilon) {
                    correct = false;
                }
            }
        }

        if (correct) {
            std::cout << "Matrix multiplication verified successfully." << std::endl;
        } else {
            std::cout << "Matrix multiplication verification failed!" << std::endl;
        }
    }

    void benchmark() {
        double startTime, endTime;
        
        startTime = omp_get_wtime();
        multiplySequential();
        endTime = omp_get_wtime();
        std::cout << "Sequential execution time: " << (endTime - startTime) << " seconds" << std::endl;

        startTime = omp_get_wtime();
        multiplyParallel();
        endTime = omp_get_wtime();
        std::cout << "Parallel execution time: " << (endTime - startTime) << " seconds" << std::endl;
    }
};

int main() {
    const int MATRIX_SIZE = 500;
    
    std::cout << "Initializing matrices of size " << MATRIX_SIZE << "x" << MATRIX_SIZE << std::endl;
    ParallelMatrixMultiplier multiplier(MATRIX_SIZE);
    
    std::cout << "Benchmarking matrix multiplication..." << std::endl;
    multiplier.benchmark();
    
    std::cout << "Verifying parallel multiplication result..." << std::endl;
    multiplier.verifyMultiplication();
    
    return 0;
}