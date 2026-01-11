
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
    size_t size;

    void initializeMatrices() {
        matrixA.resize(size, std::vector<double>(size));
        matrixB.resize(size, std::vector<double>(size));
        result.resize(size, std::vector<double>(size, 0.0));

        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < size; ++i) {
            for (size_t j = 0; j < size; ++j) {
                matrixA[i][j] = static_cast<double>(rand()) / RAND_MAX;
                matrixB[i][j] = static_cast<double>(rand()) / RAND_MAX;
            }
        }
    }

public:
    ParallelMatrixMultiplier(size_t n) : size(n) {
        srand(static_cast<unsigned>(time(nullptr)));
        initializeMatrices();
    }

    void multiply() {
        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < size; ++i) {
            for (size_t j = 0; j < size; ++j) {
                double sum = 0.0;
                #pragma omp simd reduction(+:sum)
                for (size_t k = 0; k < size; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                result[i][j] = sum;
            }
        }
    }

    void verifyWithSequential() {
        std::vector<std::vector<double>> sequentialResult(size, std::vector<double>(size, 0.0));
        
        for (size_t i = 0; i < size; ++i) {
            for (size_t j = 0; j < size; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < size; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                sequentialResult[i][j] = sum;
            }
        }

        bool correct = true;
        const double epsilon = 1e-10;
        for (size_t i = 0; i < size && correct; ++i) {
            for (size_t j = 0; j < size && correct; ++j) {
                if (std::abs(result[i][j] - sequentialResult[i][j]) > epsilon) {
                    correct = false;
                }
            }
        }

        std::cout << "Verification: " << (correct ? "PASSED" : "FAILED") << std::endl;
    }

    void benchmark() {
        double startTime = omp_get_wtime();
        multiply();
        double endTime = omp_get_wtime();

        std::cout << "Matrix size: " << size << "x" << size << std::endl;
        std::cout << "Execution time: " << (endTime - startTime) << " seconds" << std::endl;
        std::cout << "Using " << omp_get_max_threads() << " OpenMP threads" << std::endl;
    }
};

int main() {
    const size_t MATRIX_SIZE = 512;
    
    ParallelMatrixMultiplier multiplier(MATRIX_SIZE);
    
    std::cout << "Parallel Matrix Multiplication Benchmark" << std::endl;
    std::cout << "========================================" << std::endl;
    
    multiplier.benchmark();
    multiplier.verifyWithSequential();
    
    return 0;
}