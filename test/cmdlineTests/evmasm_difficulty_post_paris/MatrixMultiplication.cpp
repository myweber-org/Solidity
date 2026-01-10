
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

class ParallelMatrixMultiplier {
private:
    std::vector<std::vector<double>> matrix_a;
    std::vector<std::vector<double>> matrix_b;
    std::vector<std::vector<double>> result;
    int size;

    void initializeMatrices() {
        matrix_a.resize(size, std::vector<double>(size));
        matrix_b.resize(size, std::vector<double>(size));
        result.resize(size, std::vector<double>(size, 0.0));

        #pragma omp parallel for collapse(2)
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                matrix_a[i][j] = static_cast<double>(rand()) / RAND_MAX;
                matrix_b[i][j] = static_cast<double>(rand()) / RAND_MAX;
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
                for (int k = 0; k < size; ++k) {
                    sum += matrix_a[i][k] * matrix_b[k][j];
                }
                result[i][j] = sum;
            }
        }
    }

    void displayResult(int limit = 5) {
        std::cout << "First " << limit << "x" << limit << " elements of result matrix:" << std::endl;
        for (int i = 0; i < std::min(limit, size); ++i) {
            for (int j = 0; j < std::min(limit, size); ++j) {
                std::cout << result[i][j] << "\t";
            }
            std::cout << std::endl;
        }
    }

    double verifyComputation() {
        double checksum = 0.0;
        #pragma omp parallel for reduction(+:checksum)
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
    
    std::cout << "Parallel Matrix Multiplication (Size: " << MATRIX_SIZE << "x" << MATRIX_SIZE << ")" << std::endl;
    
    ParallelMatrixMultiplier multiplier(MATRIX_SIZE);
    
    double start_time = omp_get_wtime();
    multiplier.multiply();
    double end_time = omp_get_wtime();
    
    std::cout << "Computation time: " << (end_time - start_time) << " seconds" << std::endl;
    
    multiplier.displayResult();
    
    double checksum = multiplier.verifyComputation();
    std::cout << "Matrix checksum: " << checksum << std::endl;
    
    return 0;
}