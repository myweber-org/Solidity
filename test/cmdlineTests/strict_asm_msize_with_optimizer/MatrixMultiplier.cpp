
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
        for (int i = 0; i < std::min(size, limit); ++i) {
            for (int j = 0; j < std::min(size, limit); ++j) {
                std::cout << result[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }

    double verifyMultiplication() {
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
    
    std::cout << "Initializing parallel matrix multiplier with size " << MATRIX_SIZE << std::endl;
    
    ParallelMatrixMultiplier multiplier(MATRIX_SIZE);
    
    double start_time = omp_get_wtime();
    multiplier.multiply();
    double end_time = omp_get_wtime();
    
    std::cout << "Parallel multiplication completed in " << (end_time - start_time) << " seconds" << std::endl;
    
    multiplier.displayResult();
    
    double checksum = multiplier.verifyMultiplication();
    std::cout << "Matrix checksum: " << checksum << std::endl;
    
    return 0;
}
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <omp.h>

class ParallelMatrixMultiplier {
public:
    static std::vector<std::vector<double>> multiply(const std::vector<std::vector<double>>& A,
                                                     const std::vector<std::vector<double>>& B) {
        size_t n = A.size();
        size_t m = B[0].size();
        size_t p = B.size();
        
        if (A[0].size() != p) {
            throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
        }
        
        std::vector<std::vector<double>> C(n, std::vector<double>(m, 0.0));
        
        #pragma omp parallel for collapse(2) schedule(dynamic)
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < m; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < p; ++k) {
                    sum += A[i][k] * B[k][j];
                }
                C[i][j] = sum;
            }
        }
        
        return C;
    }
    
    static std::vector<std::vector<double>> generateRandomMatrix(size_t rows, size_t cols) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 10.0);
        
        std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                matrix[i][j] = dis(gen);
            }
        }
        return matrix;
    }
    
    static void printMatrix(const std::vector<std::vector<double>>& matrix) {
        for (const auto& row : matrix) {
            for (double val : row) {
                std::cout << val << " ";
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    const size_t N = 500;
    const size_t M = 500;
    const size_t P = 500;
    
    try {
        auto A = ParallelMatrixMultiplier::generateRandomMatrix(N, M);
        auto B = ParallelMatrixMultiplier::generateRandomMatrix(M, P);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        auto C = ParallelMatrixMultiplier::multiply(A, B);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "Matrix multiplication completed successfully." << std::endl;
        std::cout << "Matrix dimensions: " << N << "x" << M << " * " << M << "x" << P << std::endl;
        std::cout << "Execution time: " << duration.count() << " ms" << std::endl;
        
        #ifdef _OPENMP
            std::cout << "OpenMP version: " << _OPENMP << std::endl;
            std::cout << "Max threads available: " << omp_get_max_threads() << std::endl;
        #endif
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}