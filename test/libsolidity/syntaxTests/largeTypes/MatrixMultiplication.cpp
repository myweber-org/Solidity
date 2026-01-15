
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

    void initializeRandomMatrix(std::vector<std::vector<double>>& matrix, size_t rows, size_t cols) {
        matrix.resize(rows, std::vector<double>(cols));
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                matrix[i][j] = static_cast<double>(rand()) / RAND_MAX * 100.0;
            }
        }
    }

public:
    ParallelMatrixMultiplier(size_t r_a, size_t c_a, size_t r_b, size_t c_b) 
        : rows_a(r_a), cols_a(c_a), rows_b(r_b), cols_b(c_b) {
        if (cols_a != rows_b) {
            throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
        }
        
        srand(static_cast<unsigned>(time(nullptr)));
        initializeRandomMatrix(matrix_a, rows_a, cols_a);
        initializeRandomMatrix(matrix_b, rows_b, cols_b);
        result.resize(rows_a, std::vector<double>(cols_b, 0.0));
    }

    void multiplySequential() {
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

    void multiplyParallel() {
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

    void verifyResults(const std::vector<std::vector<double>>& reference) {
        const double epsilon = 1e-9;
        for (size_t i = 0; i < rows_a; ++i) {
            for (size_t j = 0; j < cols_b; ++j) {
                if (std::abs(result[i][j] - reference[i][j]) > epsilon) {
                    std::cerr << "Result verification failed at [" << i << "][" << j << "]" << std::endl;
                    return;
                }
            }
        }
        std::cout << "Result verification passed" << std::endl;
    }

    void benchmarkMultiplication() {
        std::vector<std::vector<double>> sequential_result = result;
        
        clock_t start = clock();
        multiplySequential();
        clock_t end = clock();
        double sequential_time = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        sequential_result = result;
        
        std::fill(result.begin(), result.end(), std::vector<double>(cols_b, 0.0));
        
        start = clock();
        multiplyParallel();
        end = clock();
        double parallel_time = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        
        std::cout << "Sequential time: " << sequential_time << " seconds" << std::endl;
        std::cout << "Parallel time: " << parallel_time << " seconds" << std::endl;
        std::cout << "Speedup: " << sequential_time / parallel_time << "x" << std::endl;
        
        verifyResults(sequential_result);
    }

    void displayMatrix(const std::vector<std::vector<double>>& matrix, size_t max_rows = 5, size_t max_cols = 5) {
        size_t display_rows = std::min(max_rows, matrix.size());
        size_t display_cols = (matrix.empty()) ? 0 : std::min(max_cols, matrix[0].size());
        
        for (size_t i = 0; i < display_rows; ++i) {
            for (size_t j = 0; j < display_cols; ++j) {
                std::cout << matrix[i][j] << "\t";
            }
            if (display_cols < matrix[0].size()) std::cout << "...";
            std::cout << std::endl;
        }
        if (display_rows < matrix.size()) std::cout << "...\n" << std::endl;
    }
};

int main() {
    const size_t ROWS_A = 500;
    const size_t COLS_A = 500;
    const size_t ROWS_B = 500;
    const size_t COLS_B = 500;
    
    try {
        ParallelMatrixMultiplier multiplier(ROWS_A, COLS_A, ROWS_B, COLS_B);
        
        std::cout << "Matrix multiplication benchmark (" 
                  << ROWS_A << "x" << COLS_A << " * " 
                  << ROWS_B << "x" << COLS_B << ")" << std::endl;
        
        multiplier.benchmarkMultiplication();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}