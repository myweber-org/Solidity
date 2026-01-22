
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

    static std::vector<std::vector<double>> generateRandomMatrix(size_t rows, size_t cols) {
        std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                matrix[i][j] = static_cast<double>(rand()) / RAND_MAX * 100.0;
            }
        }
        return matrix;
    }

    static void printMatrix(const std::vector<std::vector<double>>& matrix, size_t maxRows = 5, size_t maxCols = 5) {
        size_t rows = std::min(matrix.size(), maxRows);
        size_t cols = (rows > 0) ? std::min(matrix[0].size(), maxCols) : 0;

        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                std::cout << matrix[i][j] << "\t";
            }
            if (cols < matrix[0].size()) {
                std::cout << "...";
            }
            std::cout << std::endl;
        }
        if (rows < matrix.size()) {
            std::cout << "..." << std::endl;
        }
    }
};

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    omp_set_num_threads(4);

    const size_t N = 512;
    const size_t M = 512;
    const size_t P = 512;

    try {
        std::cout << "Generating random matrices..." << std::endl;
        auto A = ParallelMatrixMultiplier::generateRandomMatrix(N, P);
        auto B = ParallelMatrixMultiplier::generateRandomMatrix(P, M);

        std::cout << "Performing parallel matrix multiplication..." << std::endl;
        double startTime = omp_get_wtime();
        auto C = ParallelMatrixMultiplier::multiply(A, B);
        double endTime = omp_get_wtime();

        std::cout << "Matrix A (first 5x5):" << std::endl;
        ParallelMatrixMultiplier::printMatrix(A);
        std::cout << "\nMatrix B (first 5x5):" << std::endl;
        ParallelMatrixMultiplier::printMatrix(B);
        std::cout << "\nResult C (first 5x5):" << std::endl;
        ParallelMatrixMultiplier::printMatrix(C);

        std::cout << "\nMultiplication completed in " << (endTime - startTime) 
                  << " seconds using " << omp_get_max_threads() << " threads." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}