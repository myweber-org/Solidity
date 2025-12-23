
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <omp.h>

class Matrix {
private:
    std::vector<std::vector<double>> data;
    size_t rows, cols;

public:
    Matrix(size_t r, size_t c) : rows(r), cols(c), data(r, std::vector<double>(c, 0.0)) {}

    void randomFill() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(1.0, 10.0);

        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                data[i][j] = dis(gen);
            }
        }
    }

    Matrix multiply(const Matrix& other) const {
        if (cols != other.rows) {
            throw std::invalid_argument("Matrix dimensions mismatch for multiplication");
        }

        Matrix result(rows, other.cols);

        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < other.cols; ++j) {
                double sum = 0.0;
                #pragma omp simd reduction(+:sum)
                for (size_t k = 0; k < cols; ++k) {
                    sum += data[i][k] * other.data[k][j];
                }
                result.data[i][j] = sum;
            }
        }

        return result;
    }

    void displayFirstElements(size_t count = 5) const {
        std::cout << "First " << count << " elements: ";
        for (size_t i = 0; i < std::min(count, rows * cols); ++i) {
            std::cout << data[i / cols][i % cols] << " ";
        }
        std::cout << std::endl;
    }
};

int main() {
    const size_t N = 1000;
    
    Matrix A(N, N);
    Matrix B(N, N);

    std::cout << "Generating random matrices..." << std::endl;
    A.randomFill();
    B.randomFill();

    std::cout << "Matrix A - ";
    A.displayFirstElements();
    std::cout << "Matrix B - ";
    B.displayFirstElements();

    std::cout << "Performing matrix multiplication..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

    Matrix C = A.multiply(B);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Result matrix C - ";
    C.displayFirstElements();
    std::cout << "Multiplication completed in " << duration.count() << " ms" << std::endl;

    return 0;
}