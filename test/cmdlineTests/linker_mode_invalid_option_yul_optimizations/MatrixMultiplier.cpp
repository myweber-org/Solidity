
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

    void randomInitialize() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);

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

    bool verifyEquality(const Matrix& other, double epsilon = 1e-9) const {
        if (rows != other.rows || cols != other.cols) return false;

        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                if (std::abs(data[i][j] - other.data[i][j]) > epsilon) {
                    return false;
                }
            }
        }
        return true;
    }

    void printDimensions() const {
        std::cout << "Matrix[" << rows << "][" << cols << "]" << std::endl;
    }
};

Matrix sequentialMultiply(const Matrix& a, const Matrix& b) {
    if (a.cols != b.rows) {
        throw std::invalid_argument("Matrix dimensions mismatch for multiplication");
    }

    Matrix result(a.rows, b.cols);

    for (size_t i = 0; i < a.rows; ++i) {
        for (size_t j = 0; j < b.cols; ++j) {
            double sum = 0.0;
            for (size_t k = 0; k < a.cols; ++k) {
                sum += a.data[i][k] * b.data[k][j];
            }
            result.data[i][j] = sum;
        }
    }

    return result;
}

int main() {
    const size_t N = 512;
    
    std::cout << "Initializing matrices..." << std::endl;
    Matrix A(N, N);
    Matrix B(N, N);
    A.randomInitialize();
    B.randomInitialize();

    std::cout << "Performing parallel multiplication..." << std::endl;
    auto startParallel = std::chrono::high_resolution_clock::now();
    Matrix C_parallel = A.multiply(B);
    auto endParallel = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> parallelDuration = endParallel - startParallel;

    std::cout << "Performing sequential multiplication..." << std::endl;
    auto startSequential = std::chrono::high_resolution_clock::now();
    Matrix C_sequential = sequentialMultiply(A, B);
    auto endSequential = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> sequentialDuration = endSequential - startSequential;

    std::cout << "\nResults:" << std::endl;
    std::cout << "Parallel execution time: " << parallelDuration.count() << " seconds" << std::endl;
    std::cout << "Sequential execution time: " << sequentialDuration.count() << " seconds" << std::endl;
    std::cout << "Speedup: " << sequentialDuration.count() / parallelDuration.count() << "x" << std::endl;

    if (C_parallel.verifyEquality(C_sequential)) {
        std::cout << "Verification: Matrices are equal" << std::endl;
    } else {
        std::cout << "Verification: Matrices are NOT equal" << std::endl;
    }

    return 0;
}