
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

std::vector<std::vector<double>> generate_random_matrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
        }
    }
    return matrix;
}

std::vector<std::vector<double>> multiply_matrices_parallel(
    const std::vector<std::vector<double>>& A,
    const std::vector<std::vector<double>>& B) {
    
    int rows_A = A.size();
    int cols_A = A[0].size();
    int cols_B = B[0].size();
    
    std::vector<std::vector<double>> result(rows_A, std::vector<double>(cols_B, 0.0));
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rows_A; ++i) {
        for (int j = 0; j < cols_B; ++j) {
            double sum = 0.0;
            for (int k = 0; k < cols_A; ++k) {
                sum += A[i][k] * B[k][j];
            }
            result[i][j] = sum;
        }
    }
    
    return result;
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    const int N = 500;
    std::cout << "Generating two " << N << "x" << N << " random matrices..." << std::endl;
    
    auto matrix_A = generate_random_matrix(N, N);
    auto matrix_B = generate_random_matrix(N, N);
    
    std::cout << "Performing parallel matrix multiplication..." << std::endl;
    double start_time = omp_get_wtime();
    
    auto result = multiply_matrices_parallel(matrix_A, matrix_B);
    
    double end_time = omp_get_wtime();
    std::cout << "Multiplication completed in " << (end_time - start_time) << " seconds." << std::endl;
    
    std::cout << "Sample element result[0][0] = " << result[0][0] << std::endl;
    
    return 0;
}
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
        matrix_a.resize(size, std::vector<double>(size));
        matrix_b.resize(size, std::vector<double>(size));
        result.resize(size, std::vector<double>(size, 0.0));

        #pragma omp parallel for collapse(2)
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                matrix_a[i][j] = static_cast<double>(std::rand()) / RAND_MAX * 100.0;
                matrix_b[i][j] = static_cast<double>(std::rand()) / RAND_MAX * 100.0;
            }
        }
    }

public:
    ParallelMatrixMultiplier(int n) : size(n) {
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
        double total_sum = 0.0;
        #pragma omp parallel for reduction(+:total_sum) collapse(2)
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                total_sum += result[i][j];
            }
        }
        std::cout << "Verification sum: " << total_sum << std::endl;
    }

    void display_performance() {
        double start_time = omp_get_wtime();
        multiply();
        double end_time = omp_get_wtime();
        
        std::cout << "Matrix size: " << size << "x" << size << std::endl;
        std::cout << "Threads used: " << omp_get_max_threads() << std::endl;
        std::cout << "Execution time: " << (end_time - start_time) << " seconds" << std::endl;
        std::cout << "Operations: " << 2LL * size * size * size << std::endl;
    }
};

int main() {
    const int MATRIX_SIZE = 512;
    
    ParallelMatrixMultiplier multiplier(MATRIX_SIZE);
    multiplier.display_performance();
    multiplier.verify_calculation();
    
    return 0;
}