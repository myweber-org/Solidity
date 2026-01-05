
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
    size_t rows_a, cols_a, rows_b, cols_b;

    void initialize_matrices() {
        srand(static_cast<unsigned>(time(nullptr)));
        
        for (size_t i = 0; i < rows_a; ++i) {
            for (size_t j = 0; j < cols_a; ++j) {
                matrix_a[i][j] = static_cast<double>(rand()) / RAND_MAX * 100.0;
            }
        }
        
        for (size_t i = 0; i < rows_b; ++i) {
            for (size_t j = 0; j < cols_b; ++j) {
                matrix_b[i][j] = static_cast<double>(rand()) / RAND_MAX * 100.0;
            }
        }
    }

public:
    ParallelMatrixMultiplier(size_t r_a, size_t c_a, size_t r_b, size_t c_b) 
        : rows_a(r_a), cols_a(c_a), rows_b(r_b), cols_b(c_b) {
        
        if (cols_a != rows_b) {
            throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
        }
        
        matrix_a.resize(rows_a, std::vector<double>(cols_a, 0.0));
        matrix_b.resize(rows_b, std::vector<double>(cols_b, 0.0));
        result.resize(rows_a, std::vector<double>(cols_b, 0.0));
        
        initialize_matrices();
    }

    void multiply_sequential() {
        for (size_t i = 0; i < rows_a; ++i) {
            for (size_t j = 0; j < cols_b; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < cols_a; ++k) {
                    sum += matrix_a[i][k] * matrix_b[k][j];
                }
                result[i][j] = sum;
            }
        }
    }

    void multiply_parallel() {
        #pragma omp parallel for collapse(2) schedule(dynamic)
        for (size_t i = 0; i < rows_a; ++i) {
            for (size_t j = 0; j < cols_b; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < cols_a; ++k) {
                    sum += matrix_a[i][k] * matrix_b[k][j];
                }
                result[i][j] = sum;
            }
        }
    }

    void display_matrix(const std::vector<std::vector<double>>& mat, size_t max_rows = 5, size_t max_cols = 5) const {
        size_t display_rows = std::min(mat.size(), max_rows);
        size_t display_cols = (mat.empty()) ? 0 : std::min(mat[0].size(), max_cols);
        
        for (size_t i = 0; i < display_rows; ++i) {
            for (size_t j = 0; j < display_cols; ++j) {
                std::cout << mat[i][j] << "\t";
            }
            if (display_cols < mat[0].size()) {
                std::cout << "...";
            }
            std::cout << std::endl;
        }
        if (display_rows < mat.size()) {
            std::cout << "..." << std::endl;
        }
    }

    void benchmark_performance() {
        double start_time, end_time;
        
        std::cout << "Matrix A dimensions: " << rows_a << "x" << cols_a << std::endl;
        std::cout << "Matrix B dimensions: " << rows_b << "x" << cols_b << std::endl;
        std::cout << "Result dimensions: " << rows_a << "x" << cols_b << std::endl;
        
        start_time = omp_get_wtime();
        multiply_sequential();
        end_time = omp_get_wtime();
        std::cout << "Sequential execution time: " << (end_time - start_time) << " seconds" << std::endl;
        
        start_time = omp_get_wtime();
        multiply_parallel();
        end_time = omp_get_wtime();
        std::cout << "Parallel execution time: " << (end_time - start_time) << " seconds" << std::endl;
        
        std::cout << "\nFirst 5x5 elements of result matrix:" << std::endl;
        display_matrix(result);
    }

    const std::vector<std::vector<double>>& get_result() const {
        return result;
    }
};

int main() {
    try {
        const size_t rows_a = 500;
        const size_t cols_a = 500;
        const size_t rows_b = 500;
        const size_t cols_b = 500;
        
        ParallelMatrixMultiplier multiplier(rows_a, cols_a, rows_b, cols_b);
        multiplier.benchmark_performance();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}