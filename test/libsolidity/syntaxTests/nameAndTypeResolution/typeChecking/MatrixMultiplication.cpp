#include <iostream>
#include <vector>

std::vector<std::vector<int>> multiplyMatrices(const std::vector<std::vector<int>>& matA,
                                               const std::vector<std::vector<int>>& matB) {
    int rowsA = matA.size();
    int colsA = matA[0].size();
    int colsB = matB[0].size();

    std::vector<std::vector<int>> result(rowsA, std::vector<int>(colsB, 0));

    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            for (int k = 0; k < colsA; ++k) {
                result[i][j] += matA[i][k] * matB[k][j];
            }
        }
    }

    return result;
}

void printMatrix(const std::vector<std::vector<int>>& matrix) {
    for (const auto& row : matrix) {
        for (int val : row) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    std::vector<std::vector<int>> matrixA = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };

    std::vector<std::vector<int>> matrixB = {
        {9, 8, 7},
        {6, 5, 4},
        {3, 2, 1}
    };

    std::vector<std::vector<int>> product = multiplyMatrices(matrixA, matrixB);

    std::cout << "Matrix A:" << std::endl;
    printMatrix(matrixA);

    std::cout << "\nMatrix B:" << std::endl;
    printMatrix(matrixB);

    std::cout << "\nProduct of A and B:" << std::endl;
    printMatrix(product);

    return 0;
}
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

std::vector<std::vector<double>> generateRandomMatrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
        }
    }
    return matrix;
}

std::vector<std::vector<double>> multiplyMatricesParallel(
    const std::vector<std::vector<double>>& A,
    const std::vector<std::vector<double>>& B) {
    
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();
    
    std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            double sum = 0.0;
            for (int k = 0; k < colsA; ++k) {
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
    std::cout << "Generating random matrices of size " << N << "x" << N << std::endl;
    
    auto matrixA = generateRandomMatrix(N, N);
    auto matrixB = generateRandomMatrix(N, N);
    
    std::cout << "Starting parallel matrix multiplication..." << std::endl;
    double startTime = omp_get_wtime();
    
    auto result = multiplyMatricesParallel(matrixA, matrixB);
    
    double endTime = omp_get_wtime();
    std::cout << "Multiplication completed in " << (endTime - startTime) << " seconds" << std::endl;
    
    return 0;
}
#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <omp.h>

void initializeMatrix(std::vector<std::vector<double>>& matrix, int size) {
    for (int i = 0; i < size; ++i) {
        matrix[i].resize(size);
        for (int j = 0; j < size; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
        }
    }
}

void multiplyMatricesParallel(const std::vector<std::vector<double>>& A,
                              const std::vector<std::vector<double>>& B,
                              std::vector<std::vector<double>>& C,
                              int size) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            double sum = 0.0;
            for (int k = 0; k < size; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

void multiplyMatricesSequential(const std::vector<std::vector<double>>& A,
                                const std::vector<std::vector<double>>& B,
                                std::vector<std::vector<double>>& C,
                                int size) {
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            double sum = 0.0;
            for (int k = 0; k < size; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

int main() {
    const int MATRIX_SIZE = 500;
    const int NUM_THREADS = 4;

    omp_set_num_threads(NUM_THREADS);

    std::vector<std::vector<double>> A(MATRIX_SIZE);
    std::vector<std::vector<double>> B(MATRIX_SIZE);
    std::vector<std::vector<double>> C_parallel(MATRIX_SIZE, std::vector<double>(MATRIX_SIZE, 0.0));
    std::vector<std::vector<double>> C_sequential(MATRIX_SIZE, std::vector<double>(MATRIX_SIZE, 0.0));

    srand(42);
    initializeMatrix(A, MATRIX_SIZE);
    initializeMatrix(B, MATRIX_SIZE);

    auto start = std::chrono::high_resolution_clock::now();
    multiplyMatricesParallel(A, B, C_parallel, MATRIX_SIZE);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> parallel_duration = end - start;

    start = std::chrono::high_resolution_clock::now();
    multiplyMatricesSequential(A, B, C_sequential, MATRIX_SIZE);
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> sequential_duration = end - start;

    bool resultsMatch = true;
    const double TOLERANCE = 1e-9;
    for (int i = 0; i < MATRIX_SIZE && resultsMatch; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            if (std::abs(C_parallel[i][j] - C_sequential[i][j]) > TOLERANCE) {
                resultsMatch = false;
                break;
            }
        }
    }

    std::cout << "Matrix size: " << MATRIX_SIZE << "x" << MATRIX_SIZE << std::endl;
    std::cout << "Number of threads: " << NUM_THREADS << std::endl;
    std::cout << "Parallel execution time: " << parallel_duration.count() << " seconds" << std::endl;
    std::cout << "Sequential execution time: " << sequential_duration.count() << " seconds" << std::endl;
    std::cout << "Speedup factor: " << sequential_duration.count() / parallel_duration.count() << std::endl;
    std::cout << "Results match: " << (resultsMatch ? "Yes" : "No") << std::endl;

    return 0;
}