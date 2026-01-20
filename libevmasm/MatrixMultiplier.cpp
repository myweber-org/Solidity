
#include <iostream>
#include <vector>
#include <omp.h>
#include <chrono>
#include <cstdlib>

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
        #pragma omp parallel for collapse(2)
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                matrixA[i][j] = static_cast<double>(rand()) / RAND_MAX;
                matrixB[i][j] = static_cast<double>(rand()) / RAND_MAX;
            }
        }
    }

    void multiplySequential() {
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                double sum = 0.0;
                for (int k = 0; k < size; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                result[i][j] = sum;
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "Sequential multiplication time: " << duration.count() << " seconds\n";
    }

    void multiplyParallel() {
        auto start = std::chrono::high_resolution_clock::now();
        
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
        
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "Parallel multiplication time: " << duration.count() << " seconds\n";
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

    void printPerformanceStats() {
        std::cout << "Matrix size: " << size << "x" << size << std::endl;
        std::cout << "Available threads: " << omp_get_max_threads() << std::endl;
    }
};

int main() {
    const int MATRIX_SIZE = 512;
    srand(static_cast<unsigned>(time(nullptr)));

    ParallelMatrixMultiplier multiplier(MATRIX_SIZE);
    
    multiplier.printPerformanceStats();
    std::cout << std::endl;

    std::cout << "Performing sequential multiplication...\n";
    multiplier.multiplySequential();
    
    std::cout << "\nPerforming parallel multiplication...\n";
    multiplier.multiplyParallel();

    return 0;
}