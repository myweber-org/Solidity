
#include <iostream>
#include <vector>
#include <chrono>
#include <omp.h>
#include <cstdlib>

class ParallelMatrixMultiplier {
public:
    static std::vector<std::vector<double>> multiply(const std::vector<std::vector<double>>& A,
                                                     const std::vector<std::vector<double>>& B) {
        size_t rowsA = A.size();
        size_t colsA = A[0].size();
        size_t rowsB = B.size();
        size_t colsB = B[0].size();
        
        if (colsA != rowsB) {
            throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
        }
        
        std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));
        
        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < colsA; ++k) {
                    sum += A[i][k] * B[k][j];
                }
                result[i][j] = sum;
            }
        }
        
        return result;
    }
    
    static void initializeRandomMatrix(std::vector<std::vector<double>>& matrix) {
        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < matrix.size(); ++i) {
            for (size_t j = 0; j < matrix[0].size(); ++j) {
                matrix[i][j] = static_cast<double>(rand()) / RAND_MAX * 100.0;
            }
        }
    }
    
    static void benchmark(size_t size) {
        std::vector<std::vector<double>> A(size, std::vector<double>(size));
        std::vector<std::vector<double>> B(size, std::vector<double>(size));
        
        initializeRandomMatrix(A);
        initializeRandomMatrix(B);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        auto result = multiply(A, B);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "Matrix multiplication of size " << size << "x" << size 
                  << " completed in " << duration.count() << " ms" << std::endl;
        
        double checksum = 0.0;
        for (const auto& row : result) {
            for (double val : row) {
                checksum += val;
            }
        }
        std::cout << "Result checksum: " << checksum << std::endl;
    }
};

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    std::cout << "OpenMP Matrix Multiplication Benchmark" << std::endl;
    std::cout << "Available threads: " << omp_get_max_threads() << std::endl;
    
    size_t sizes[] = {256, 512, 1024};
    
    for (size_t size : sizes) {
        ParallelMatrixMultiplier::benchmark(size);
    }
    
    return 0;
}