
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

    bool verifyResult(const std::vector<std::vector<double>>& reference) {
        const double epsilon = 1e-10;
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                if (std::abs(result[i][j] - reference[i][j]) > epsilon) {
                    return false;
                }
            }
        }
        return true;
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
        
        bool correct = verifyResult(sequentialResult);
        
        std::cout << "Matrix size: " << size << "x" << size << std::endl;
        std::cout << "Sequential time: " << seqTime << " seconds" << std::endl;
        std::cout << "Parallel time: " << parTime << " seconds" << std::endl;
        std::cout << "Speedup: " << seqTime / parTime << std::endl;
        std::cout << "Result correct: " << (correct ? "Yes" : "No") << std::endl;
    }
};

int main() {
    const int matrixSize = 500;
    ParallelMatrixMultiplier multiplier(matrixSize);
    multiplier.benchmark();
    return 0;
}