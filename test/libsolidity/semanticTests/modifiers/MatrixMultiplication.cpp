
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

    void initialize_matrices() {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        
        #pragma omp parallel for collapse(2)
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                matrix_a[i][j] = static_cast<double>(std::rand()) / RAND_MAX * 100.0;
                matrix_b[i][j] = static_cast<double>(std::rand()) / RAND_MAX * 100.0;
                result[i][j] = 0.0;
            }
        }
    }

public:
    ParallelMatrixMultiplier(int n) : size(n) {
        matrix_a.resize(size, std::vector<double>(size));
        matrix_b.resize(size, std::vector<double>(size));
        result.resize(size, std::vector<double>(size));
        initialize_matrices();
    }

    void multiply() {
        #pragma omp parallel for collapse(2)
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                double sum = 0.0;
                #pragma omp simd reduction(+:sum)
                for (int k = 0; k < size; ++k) {
                    sum += matrix_a[i][k] * matrix_b[k][j];
                }
                result[i][j] = sum;
            }
        }
    }

    void verify_calculation() {
        std::vector<std::vector<double>> sequential_result(size, std::vector<double>(size, 0.0));
        
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                for (int k = 0; k < size; ++k) {
                    sequential_result[i][j] += matrix_a[i][k] * matrix_b[k][j];
                }
            }
        }

        bool correct = true;
        const double epsilon = 1e-9;
        
        for (int i = 0; i < size && correct; ++i) {
            for (int j = 0; j < size && correct; ++j) {
                if (std::abs(result[i][j] - sequential_result[i][j]) > epsilon) {
                    correct = false;
                }
            }
        }

        std::cout << "Verification: " << (correct ? "PASSED" : "FAILED") << std::endl;
    }

    void benchmark_performance(int iterations = 5) {
        double total_time = 0.0;
        
        for (int iter = 0; iter < iterations; ++iter) {
            double start_time = omp_get_wtime();
            multiply();
            double end_time = omp_get_wtime();
            total_time += (end_time - start_time);
            
            if (iter == 0) {
                verify_calculation();
            }
        }
        
        std::cout << "Average execution time over " << iterations 
                  << " iterations: " << total_time / iterations 
                  << " seconds" << std::endl;
    }

    void display_matrix_info() {
        std::cout << "Matrix size: " << size << "x" << size << std::endl;
        std::cout << "Total elements per matrix: " << size * size << std::endl;
        std::cout << "OpenMP threads available: " << omp_get_max_threads() << std::endl;
    }
};

int main() {
    const int matrix_size = 512;
    
    ParallelMatrixMultiplier multiplier(matrix_size);
    
    multiplier.display_matrix_info();
    multiplier.benchmark_performance();
    
    return 0;
}