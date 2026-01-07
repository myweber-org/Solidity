
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
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
                #pragma omp simd reduction(+:sum)
                for (size_t k = 0; k < p; ++k) {
                    sum += A[i][k] * B[k][j];
                }
                C[i][j] = sum;
            }
        }
        
        return C;
    }
    
    static void initializeRandomMatrix(std::vector<std::vector<double>>& matrix) {
        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < matrix.size(); ++i) {
            for (size_t j = 0; j < matrix[0].size(); ++j) {
                matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
            }
        }
    }
    
    static bool verifyMultiplication(const std::vector<std::vector<double>>& A,
                                     const std::vector<std::vector<double>>& B,
                                     const std::vector<std::vector<double>>& C,
                                     double tolerance = 1e-6) {
        size_t n = A.size();
        size_t m = B[0].size();
        size_t p = B.size();
        
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < m; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < p; ++k) {
                    sum += A[i][k] * B[k][j];
                }
                if (std::abs(C[i][j] - sum) > tolerance) {
                    return false;
                }
            }
        }
        return true;
    }
};

int main() {
    const size_t N = 512;
    srand(static_cast<unsigned>(time(nullptr)));
    
    std::vector<std::vector<double>> A(N, std::vector<double>(N));
    std::vector<std::vector<double>> B(N, std::vector<double>(N));
    
    ParallelMatrixMultiplier::initializeRandomMatrix(A);
    ParallelMatrixMultiplier::initializeRandomMatrix(B);
    
    double start_time = omp_get_wtime();
    auto C = ParallelMatrixMultiplier::multiply(A, B);
    double end_time = omp_get_wtime();
    
    std::cout << "Matrix multiplication completed in " << (end_time - start_time) << " seconds\n";
    
    if (ParallelMatrixMultiplier::verifyMultiplication(A, B, C)) {
        std::cout << "Result verification passed\n";
    } else {
        std::cout << "Result verification failed\n";
    }
    
    std::cout << "First element of result: " << C[0][0] << "\n";
    
    return 0;
}