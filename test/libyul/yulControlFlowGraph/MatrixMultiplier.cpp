
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
        srand(static_cast<unsigned>(time(nullptr)));
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

    void displayResult(int limit = 5) const {
        int displaySize = std::min(size, limit);
        std::cout << "First " << displaySize << "x" << displaySize << " elements of result matrix:\n";
        for (int i = 0; i < displaySize; ++i) {
            for (int j = 0; j < displaySize; ++j) {
                std::cout << result[i][j] << "\t";
            }
            std::cout << "\n";
        }
    }

    double verifyMultiplication() const {
        double checksum = 0.0;
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                checksum += result[i][j];
            }
        }
        return checksum;
    }
};

int main() {
    const int MATRIX_SIZE = 512;
    
    std::cout << "Initializing parallel matrix multiplier with size " << MATRIX_SIZE << "x" << MATRIX_SIZE << "...\n";
    
    ParallelMatrixMultiplier multiplier(MATRIX_SIZE);
    
    std::cout << "Performing parallel matrix multiplication...\n";
    double startTime = omp_get_wtime();
    multiplier.multiply();
    double endTime = omp_get_wtime();
    
    std::cout << "Multiplication completed in " << (endTime - startTime) << " seconds.\n";
    
    multiplier.displayResult();
    
    double checksum = multiplier.verifyMultiplication();
    std::cout << "Matrix checksum: " << checksum << "\n";
    
    return 0;
}