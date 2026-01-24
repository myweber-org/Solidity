
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
        #pragma omp parallel for
        for (size_t i = 0; i < matrix.size(); ++i) {
            for (size_t j = 0; j < matrix[0].size(); ++j) {
                matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
            }
        }
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
    auto result = ParallelMatrixMultiplier::multiply(matrixA, matrixB);
    double endTime = omp_get_wtime();
    
    std::cout << "Matrix multiplication completed for " << N << "x" << N << " matrices." << std::endl;
    std::cout << "Execution time: " << (endTime - startTime) << " seconds" << std::endl;
    
    double checksum = 0.0;
    for (size_t i = 0; i < std::min(N, static_cast<size_t>(10)); ++i) {
        for (size_t j = 0; j < std::min(N, static_cast<size_t>(10)); ++j) {
            checksum += result[i][j];
        }
    }
    std::cout << "Checksum of first 10x10 elements: " << checksum << std::endl;
    
    return 0;
}#include <iostream>
#include <vector>
#include <stdexcept>

class MatrixMultiplier {
public:
    static std::vector<std::vector<double>> multiply(const std::vector<std::vector<double>>& matrixA,
                                                     const std::vector<std::vector<double>>& matrixB) {
        size_t rowsA = matrixA.size();
        if (rowsA == 0) throw std::invalid_argument("Matrix A is empty");
        size_t colsA = matrixA[0].size();
        size_t rowsB = matrixB.size();
        if (rowsB == 0) throw std::invalid_argument("Matrix B is empty");
        size_t colsB = matrixB[0].size();

        for (size_t i = 1; i < rowsA; ++i) {
            if (matrixA[i].size() != colsA) {
                throw std::invalid_argument("Matrix A has inconsistent row sizes");
            }
        }
        for (size_t i = 1; i < rowsB; ++i) {
            if (matrixB[i].size() != colsB) {
                throw std::invalid_argument("Matrix B has inconsistent row sizes");
            }
        }

        if (colsA != rowsB) {
            throw std::invalid_argument("Matrix dimensions mismatch for multiplication");
        }

        std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));

        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                for (size_t k = 0; k < colsA; ++k) {
                    result[i][j] += matrixA[i][k] * matrixB[k][j];
                }
            }
        }

        return result;
    }

    static void printMatrix(const std::vector<std::vector<double>>& matrix) {
        for (const auto& row : matrix) {
            for (double val : row) {
                std::cout << val << " ";
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    try {
        std::vector<std::vector<double>> A = {{1, 2, 3}, {4, 5, 6}};
        std::vector<std::vector<double>> B = {{7, 8}, {9, 10}, {11, 12}};

        std::cout << "Matrix A:" << std::endl;
        MatrixMultiplier::printMatrix(A);
        std::cout << "Matrix B:" << std::endl;
        MatrixMultiplier::printMatrix(B);

        auto result = MatrixMultiplier::multiply(A, B);

        std::cout << "Result of A * B:" << std::endl;
        MatrixMultiplier::printMatrix(result);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
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

vector<vector<double>> multiplyMatricesSequential(const vector<vector<double>>& A,
                                                  const vector<vector<double>>& B) {
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();
    
    vector<vector<double>> result(rowsA, vector<double>(colsB, 0.0));
    
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

bool verifyResults(const vector<vector<double>>& seqResult,
                   const vector<vector<double>>& parResult,
                   double tolerance = 1e-10) {
    if (seqResult.size() != parResult.size()) return false;
    
    for (size_t i = 0; i < seqResult.size(); ++i) {
        if (seqResult[i].size() != parResult[i].size()) return false;
        
        for (size_t j = 0; j < seqResult[i].size(); ++j) {
            if (abs(seqResult[i][j] - parResult[i][j]) > tolerance) {
                return false;
            }
        }
    }
    return true;
}

int main() {
    const int SIZE = 500;
    
    cout << "Generating random matrices of size " << SIZE << "x" << SIZE << "..." << endl;
    auto A = generateRandomMatrix(SIZE, SIZE);
    auto B = generateRandomMatrix(SIZE, SIZE);
    
    cout << "Performing sequential matrix multiplication..." << endl;
    auto start = chrono::high_resolution_clock::now();
    auto seqResult = multiplyMatricesSequential(A, B);
    auto end = chrono::high_resolution_clock::now();
    auto seqDuration = chrono::duration_cast<chrono::milliseconds>(end - start);
    
    cout << "Performing parallel matrix multiplication..." << endl;
    start = chrono::high_resolution_clock::now();
    auto parResult = multiplyMatricesParallel(A, B);
    end = chrono::high_resolution_clock::now();
    auto parDuration = chrono::duration_cast<chrono::milliseconds>(end - start);
    
    cout << "\nPerformance Results:" << endl;
    cout << "Sequential time: " << seqDuration.count() << " ms" << endl;
    cout << "Parallel time: " << parDuration.count() << " ms" << endl;
    cout << "Speedup: " << static_cast<double>(seqDuration.count()) / parDuration.count() << "x" << endl;
    
    cout << "\nVerifying results..." << endl;
    if (verifyResults(seqResult, parResult)) {
        cout << "Results match! Parallel implementation is correct." << endl;
    } else {
        cout << "ERROR: Results do not match!" << endl;
    }
    
    return 0;
}