
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
        #pragma omp parallel for collapse(2)
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                matrixA[i][j] = static_cast<double>(rand()) / RAND_MAX;
                matrixB[i][j] = static_cast<double>(rand()) / RAND_MAX;
            }
        }
    }

    void multiply() {
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

    void displayResult(int limit = 5) {
        std::cout << "First " << limit << "x" << limit << " elements of result matrix:" << std::endl;
        for (int i = 0; i < std::min(limit, size); ++i) {
            for (int j = 0; j < std::min(limit, size); ++j) {
                std::cout << result[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }

    double verifyMultiplication() {
        double checksum = 0.0;
        #pragma omp parallel for reduction(+:checksum) collapse(2)
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                checksum += result[i][j];
            }
        }
        return checksum;
    }
};

int main() {
    const int MATRIX_SIZE = 500;
    
    std::cout << "Initializing parallel matrix multiplier with size " 
              << MATRIX_SIZE << "x" << MATRIX_SIZE << std::endl;
    
    ParallelMatrixMultiplier multiplier(MATRIX_SIZE);
    
    double start_time = omp_get_wtime();
    multiplier.multiply();
    double end_time = omp_get_wtime();
    
    std::cout << "Matrix multiplication completed in " 
              << (end_time - start_time) << " seconds" << std::endl;
    
    multiplier.displayResult();
    
    double checksum = multiplier.verifyMultiplication();
    std::cout << "Verification checksum: " << checksum << std::endl;
    
    return 0;
}