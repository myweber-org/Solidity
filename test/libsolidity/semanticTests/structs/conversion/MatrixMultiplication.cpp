
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

    void initialize_matrix(std::vector<std::vector<double>>& matrix) {
        #pragma omp parallel for collapse(2)
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
            }
        }
    }

public:
    ParallelMatrixMultiplier(int n) : size(n) {
        matrix_a.resize(size, std::vector<double>(size));
        matrix_b.resize(size, std::vector<double>(size));
        result.resize(size, std::vector<double>(size, 0.0));
        
        srand(static_cast<unsigned>(time(nullptr)));
        initialize_matrix(matrix_a);
        initialize_matrix(matrix_b);
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

        double max_error = 0.0;
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                double error = std::abs(result[i][j] - sequential_result[i][j]);
                if (error > max_error) {
                    max_error = error;
                }
            }
        }

        std::cout << "Verification complete. Maximum error: " << max_error << std::endl;
    }

    void display_performance() {
        double start_time = omp_get_wtime();
        multiply();
        double end_time = omp_get_wtime();
        
        std::cout << "Matrix size: " << size << "x" << size << std::endl;
        std::cout << "Execution time: " << (end_time - start_time) << " seconds" << std::endl;
        std::cout << "Threads used: " << omp_get_max_threads() << std::endl;
    }
};

int main() {
    const int MATRIX_SIZE = 512;
    
    ParallelMatrixMultiplier multiplier(MATRIX_SIZE);
    
    multiplier.display_performance();
    multiplier.verify_calculation();
    
    return 0;
}