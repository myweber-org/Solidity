
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
        matrix_a.resize(rows_a, std::vector<double>(cols_a));
        matrix_b.resize(rows_b, std::vector<double>(cols_b));
        result.resize(rows_a, std::vector<double>(cols_b, 0.0));

        std::srand(static_cast<unsigned>(std::time(nullptr)));
        for (size_t i = 0; i < rows_a; ++i) {
            for (size_t j = 0; j < cols_a; ++j) {
                matrix_a[i][j] = static_cast<double>(std::rand()) / RAND_MAX;
            }
        }
        for (size_t i = 0; i < rows_b; ++i) {
            for (size_t j = 0; j < cols_b; ++j) {
                matrix_b[i][j] = static_cast<double>(std::rand()) / RAND_MAX;
            }
        }
    }

public:
    ParallelMatrixMultiplier(size_t r_a, size_t c_a, size_t r_b, size_t c_b)
        : rows_a(r_a), cols_a(c_a), rows_b(r_b), cols_b(c_b) {
        if (cols_a != rows_b) {
            throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
        }
        initialize_matrices();
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

    void verify_with_sequential() {
        std::vector<std::vector<double>> sequential_result(rows_a, std::vector<double>(cols_b, 0.0));
        for (size_t i = 0; i < rows_a; ++i) {
            for (size_t j = 0; j < cols_b; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < cols_a; ++k) {
                    sum += matrix_a[i][k] * matrix_b[k][j];
                }
                sequential_result[i][j] = sum;
            }
        }

        bool correct = true;
        const double epsilon = 1e-9;
        for (size_t i = 0; i < rows_a && correct; ++i) {
            for (size_t j = 0; j < cols_b && correct; ++j) {
                if (std::abs(result[i][j] - sequential_result[i][j]) > epsilon) {
                    correct = false;
                }
            }
        }
        std::cout << "Verification: " << (correct ? "PASSED" : "FAILED") << std::endl;
    }

    void print_result_dimensions() const {
        std::cout << "Result matrix dimensions: " << rows_a << " x " << cols_b << std::endl;
    }

    double get_element(size_t i, size_t j) const {
        if (i >= rows_a || j >= cols_b) {
            throw std::out_of_range("Index out of bounds");
        }
        return result[i][j];
    }
};

int main() {
    const size_t rows = 500;
    const size_t cols = 500;

    try {
        ParallelMatrixMultiplier multiplier(rows, cols, cols, rows);
        
        double start_time = omp_get_wtime();
        multiplier.multiply_parallel();
        double end_time = omp_get_wtime();

        multiplier.print_result_dimensions();
        std::cout << "Parallel multiplication time: " << (end_time - start_time) << " seconds" << std::endl;

        multiplier.verify_with_sequential();

        std::cout << "Sample element (0,0): " << multiplier.get_element(0, 0) << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}