
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <omp.h>

std::vector<std::vector<double>> generateRandomMatrix(int rows, int cols) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 100.0);
    
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = dis(gen);
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

std::vector<std::vector<double>> multiplyMatricesSequential(
    const std::vector<std::vector<double>>& A,
    const std::vector<std::vector<double>>& B) {
    
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();
    
    std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));
    
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

bool verifyResults(const std::vector<std::vector<double>>& result1,
                   const std::vector<std::vector<double>>& result2,
                   double tolerance = 1e-6) {
    
    if (result1.size() != result2.size() || result1[0].size() != result2[0].size()) {
        return false;
    }
    
    for (size_t i = 0; i < result1.size(); ++i) {
        for (size_t j = 0; j < result1[0].size(); ++j) {
            if (std::abs(result1[i][j] - result2[i][j]) > tolerance) {
                return false;
            }
        }
    }
    return true;
}

int main() {
    const int ROWS_A = 500;
    const int COLS_A = 500;
    const int COLS_B = 500;
    
    std::cout << "Generating random matrices..." << std::endl;
    auto matrixA = generateRandomMatrix(ROWS_A, COLS_A);
    auto matrixB = generateRandomMatrix(COLS_A, COLS_B);
    
    std::cout << "Performing sequential matrix multiplication..." << std::endl;
    auto startSeq = std::chrono::high_resolution_clock::now();
    auto resultSeq = multiplyMatricesSequential(matrixA, matrixB);
    auto endSeq = std::chrono::high_resolution_clock::now();
    auto durationSeq = std::chrono::duration_cast<std::chrono::milliseconds>(endSeq - startSeq);
    
    std::cout << "Performing parallel matrix multiplication..." << std::endl;
    auto startPar = std::chrono::high_resolution_clock::now();
    auto resultPar = multiplyMatricesParallel(matrixA, matrixB);
    auto endPar = std::chrono::high_resolution_clock::now();
    auto durationPar = std::chrono::duration_cast<std::chrono::milliseconds>(endPar - startPar);
    
    std::cout << "\nPerformance Results:" << std::endl;
    std::cout << "Matrix dimensions: A[" << ROWS_A << "x" << COLS_A 
              << "] * B[" << COLS_A << "x" << COLS_B << "]" << std::endl;
    std::cout << "Sequential time: " << durationSeq.count() << " ms" << std::endl;
    std::cout << "Parallel time: " << durationPar.count() << " ms" << std::endl;
    std::cout << "Speedup: " << static_cast<double>(durationSeq.count()) / durationPar.count() 
              << "x" << std::endl;
    
    std::cout << "\nVerifying results..." << std::endl;
    if (verifyResults(resultSeq, resultPar)) {
        std::cout << "Results match! Parallel implementation is correct." << std::endl;
    } else {
        std::cout << "ERROR: Results do not match!" << std::endl;
    }
    
    return 0;
}
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

using namespace std;

vector<vector<double>> generateRandomMatrix(int rows, int cols) {
    vector<vector<double>> matrix(rows, vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
        }
    }
    return matrix;
}

vector<vector<double>> multiplyMatricesParallel(const vector<vector<double>>& A,
                                                const vector<vector<double>>& B) {
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();
    
    vector<vector<double>> result(rowsA, vector<double>(colsB, 0.0));
    
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
    srand(static_cast<unsigned>(time(0)));
    
    const int N = 500;
    cout << "Generating " << N << "x" << N << " matrices..." << endl;
    
    auto matrixA = generateRandomMatrix(N, N);
    auto matrixB = generateRandomMatrix(N, N);
    
    cout << "Performing parallel matrix multiplication..." << endl;
    double startTime = omp_get_wtime();
    
    auto result = multiplyMatricesParallel(matrixA, matrixB);
    
    double endTime = omp_get_wtime();
    cout << "Multiplication completed in " << (endTime - startTime) << " seconds" << endl;
    
    return 0;
}#include <iostream>
#include <array>

template<typename T, size_t N>
using Matrix = std::array<std::array<T, N>, N>;

template<typename T, size_t N>
Matrix<T, N> multiplyMatrices(const Matrix<T, N>& a, const Matrix<T, N>& b) {
    Matrix<T, N> result{};
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            T sum = 0;
            for (size_t k = 0; k < N; ++k) {
                sum += a[i][k] * b[k][j];
            }
            result[i][j] = sum;
        }
    }
    return result;
}

template<typename T, size_t N>
void printMatrix(const Matrix<T, N>& m) {
    for (const auto& row : m) {
        for (const auto& elem : row) {
            std::cout << elem << ' ';
        }
        std::cout << '\n';
    }
}

int main() {
    constexpr size_t SIZE = 3;
    Matrix<int, SIZE> matA = {{
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    }};
    
    Matrix<int, SIZE> matB = {{
        {9, 8, 7},
        {6, 5, 4},
        {3, 2, 1}
    }};
    
    std::cout << "Matrix A:\n";
    printMatrix(matA);
    
    std::cout << "\nMatrix B:\n";
    printMatrix(matB);
    
    Matrix<int, SIZE> product = multiplyMatrices(matA, matB);
    
    std::cout << "\nProduct A x B:\n";
    printMatrix(product);
    
    return 0;
}