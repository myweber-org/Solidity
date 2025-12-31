
#include <iostream>
#include <vector>
#include <chrono>
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

int main() {
    const int SIZE = 500;
    
    std::cout << "Generating random matrices of size " << SIZE << "x" << SIZE << std::endl;
    auto A = generateRandomMatrix(SIZE, SIZE);
    auto B = generateRandomMatrix(SIZE, SIZE);
    
    std::cout << "Performing sequential multiplication..." << std::endl;
    auto startSeq = std::chrono::high_resolution_clock::now();
    auto resultSeq = multiplyMatricesSequential(A, B);
    auto endSeq = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> seqDuration = endSeq - startSeq;
    
    std::cout << "Performing parallel multiplication..." << std::endl;
    auto startPar = std::chrono::high_resolution_clock::now();
    auto resultPar = multiplyMatricesParallel(A, B);
    auto endPar = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> parDuration = endPar - startPar;
    
    std::cout << "Sequential execution time: " << seqDuration.count() << " seconds" << std::endl;
    std::cout << "Parallel execution time: " << parDuration.count() << " seconds" << std::endl;
    std::cout << "Speedup factor: " << seqDuration.count() / parDuration.count() << std::endl;
    
    bool resultsMatch = true;
    double tolerance = 1e-10;
    for (int i = 0; i < SIZE && resultsMatch; ++i) {
        for (int j = 0; j < SIZE && resultsMatch; ++j) {
            if (std::abs(resultSeq[i][j] - resultPar[i][j]) > tolerance) {
                resultsMatch = false;
            }
        }
    }
    
    if (resultsMatch) {
        std::cout << "Results verification: PASSED" << std::endl;
    } else {
        std::cout << "Results verification: FAILED" << std::endl;
    }
    
    return 0;
}
#include <iostream>
#include <vector>

std::vector<std::vector<int>> multiplyMatrices(const std::vector<std::vector<int>>& A,
                                               const std::vector<std::vector<int>>& B) {
    int rowsA = A.size();
    int colsA = A[0].size();
    int rowsB = B.size();
    int colsB = B[0].size();

    if (colsA != rowsB) {
        throw std::invalid_argument("Matrix dimensions are not compatible for multiplication.");
    }

    std::vector<std::vector<int>> result(rowsA, std::vector<int>(colsB, 0));

    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            for (int k = 0; k < colsA; ++k) {
                result[i][j] += A[i][k] * B[k][j];
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
    std::vector<std::vector<int>> matrixA = {{1, 2, 3}, {4, 5, 6}};
    std::vector<std::vector<int>> matrixB = {{7, 8}, {9, 10}, {11, 12}};

    try {
        std::vector<std::vector<int>> product = multiplyMatrices(matrixA, matrixB);
        std::cout << "Result of matrix multiplication:" << std::endl;
        printMatrix(product);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}