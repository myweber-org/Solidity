
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

    void verifyResult(const std::vector<std::vector<double>>& reference) {
        double tolerance = 1e-10;
        bool correct = true;
        
        for (int i = 0; i < size && correct; ++i) {
            for (int j = 0; j < size && correct; ++j) {
                if (std::abs(result[i][j] - reference[i][j]) > tolerance) {
                    correct = false;
                }
            }
        }
        
        std::cout << "Result verification: " 
                  << (correct ? "PASSED" : "FAILED") << std::endl;
    }

    void benchmark() {
        std::vector<std::vector<double>> sequentialResult(size, std::vector<double>(size));
        
        double start = omp_get_wtime();
        multiplySequential();
        double seqTime = omp_get_wtime() - start;
        sequentialResult = result;
        
        std::fill(result.begin(), result.end(), std::vector<double>(size, 0.0));
        
        start = omp_get_wtime();
        multiplyParallel();
        double parTime = omp_get_wtime() - start;
        
        std::cout << "Matrix size: " << size << "x" << size << std::endl;
        std::cout << "Sequential time: " << seqTime << " seconds" << std::endl;
        std::cout << "Parallel time: " << parTime << " seconds" << std::endl;
        std::cout << "Speedup: " << seqTime / parTime << "x" << std::endl;
        
        verifyResult(sequentialResult);
    }
};

int main() {
    const int MATRIX_SIZE = 500;
    
    ParallelMatrixMultiplier multiplier(MATRIX_SIZE);
    multiplier.benchmark();
    
    return 0;
}