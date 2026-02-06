
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

class ParallelMatrixMultiplier {
public:
    static std::vector<std::vector<double>> multiply(const std::vector<std::vector<double>>& A,
                                                     const std::vector<std::vector<double>>& B) {
        size_t rowsA = A.size();
        size_t colsA = A[0].size();
        size_t colsB = B[0].size();
        
        std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));
        
        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < colsA; ++k) {
                    sum += A[i][k] * B[k][j];
                }
                result[i][j] = sum;
            }
        }
        
        return result;
    }
    
    static void initializeRandomMatrix(std::vector<std::vector<double>>& matrix) {
        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < matrix.size(); ++i) {
            for (size_t j = 0; j < matrix[0].size(); ++j) {
                matrix[i][j] = static_cast<double>(rand()) / RAND_MAX * 100.0;
            }
        }
    }
    
    static bool validateMultiplication(const std::vector<std::vector<double>>& A,
                                       const std::vector<std::vector<double>>& B,
                                       const std::vector<std::vector<double>>& C) {
        size_t rowsA = A.size();
        size_t colsA = A[0].size();
        size_t colsB = B[0].size();
        
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < colsA; ++k) {
                    sum += A[i][k] * B[k][j];
                }
                if (std::abs(sum - C[i][j]) > 1e-6) {
                    return false;
                }
            }
        }
        return true;
    }
};

int main() {
    const size_t N = 512;
    srand(static_cast<unsigned>(time(nullptr)));
    
    std::vector<std::vector<double>> matrixA(N, std::vector<double>(N));
    std::vector<std::vector<double>> matrixB(N, std::vector<double>(N));
    
    ParallelMatrixMultiplier::initializeRandomMatrix(matrixA);
    ParallelMatrixMultiplier::initializeRandomMatrix(matrixB);
    
    double startTime = omp_get_wtime();
    std::vector<std::vector<double>> result = 
        ParallelMatrixMultiplier::multiply(matrixA, matrixB);
    double endTime = omp_get_wtime();
    
    bool isValid = ParallelMatrixMultiplier::validateMultiplication(matrixA, matrixB, result);
    
    std::cout << "Matrix multiplication completed." << std::endl;
    std::cout << "Matrix size: " << N << "x" << N << std::endl;
    std::cout << "Execution time: " << (endTime - startTime) << " seconds" << std::endl;
    std::cout << "Validation result: " << (isValid ? "PASSED" : "FAILED") << std::endl;
    
    return 0;
}#include <iostream>
#include <vector>

std::vector<std::vector<int>> multiplyMatrices(const std::vector<std::vector<int>>& A,
                                               const std::vector<std::vector<int>>& B) {
    int rowsA = A.size();
    int colsA = A[0].size();
    int rowsB = B.size();
    int colsB = B[0].size();

    if (colsA != rowsB) {
        throw std::invalid_argument("Matrix dimensions are incompatible for multiplication.");
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
    std::vector<std::vector<int>> matrixA = {{1, 2, 3},
                                             {4, 5, 6}};

    std::vector<std::vector<int>> matrixB = {{7, 8},
                                             {9, 10},
                                             {11, 12}};

    try {
        std::vector<std::vector<int>> product = multiplyMatrices(matrixA, matrixB);
        std::cout << "Result of matrix multiplication:" << std::endl;
        printMatrix(product);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
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

vector<vector<double>> multiplyMatrices(const vector<vector<double>>& A,
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

void printMatrixStats(const vector<vector<double>>& matrix, const string& name) {
    double sum = 0.0;
    double minVal = matrix[0][0];
    double maxVal = matrix[0][0];
    
    #pragma omp parallel for reduction(+:sum) reduction(min:minVal) reduction(max:maxVal) collapse(2)
    for (size_t i = 0; i < matrix.size(); ++i) {
        for (size_t j = 0; j < matrix[0].size(); ++j) {
            double val = matrix[i][j];
            sum += val;
            if (val < minVal) minVal = val;
            if (val > maxVal) maxVal = val;
        }
    }
    
    cout << name << " - Sum: " << sum 
         << ", Min: " << minVal 
         << ", Max: " << maxVal 
         << ", Avg: " << sum / (matrix.size() * matrix[0].size()) << endl;
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    const int SIZE = 500;
    
    cout << "Generating random matrices of size " << SIZE << "x" << SIZE << "..." << endl;
    auto matrixA = generateRandomMatrix(SIZE, SIZE);
    auto matrixB = generateRandomMatrix(SIZE, SIZE);
    
    cout << "Multiplying matrices using OpenMP parallelization..." << endl;
    double startTime = omp_get_wtime();
    
    auto result = multiplyMatrices(matrixA, matrixB);
    
    double endTime = omp_get_wtime();
    double executionTime = endTime - startTime;
    
    cout << "Matrix multiplication completed in " << executionTime << " seconds" << endl;
    cout << "Performance: " << (2.0 * SIZE * SIZE * SIZE) / (executionTime * 1e9) << " GFLOPS" << endl;
    
    printMatrixStats(matrixA, "Matrix A");
    printMatrixStats(matrixB, "Matrix B");
    printMatrixStats(result, "Result Matrix");
    
    return 0;
}