
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

using namespace std;

vector<vector<int>> generateRandomMatrix(int rows, int cols) {
    vector<vector<int>> matrix(rows, vector<int>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = rand() % 100;
        }
    }
    return matrix;
}

vector<vector<int>> multiplyMatricesParallel(const vector<vector<int>>& A,
                                             const vector<vector<int>>& B) {
    int m = A.size();
    int n = A[0].size();
    int p = B[0].size();
    
    vector<vector<int>> result(m, vector<int>(p, 0));
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < p; ++j) {
            int sum = 0;
            for (int k = 0; k < n; ++k) {
                sum += A[i][k] * B[k][j];
            }
            result[i][j] = sum;
        }
    }
    
    return result;
}

int main() {
    srand(time(nullptr));
    
    const int SIZE = 500;
    
    auto matrixA = generateRandomMatrix(SIZE, SIZE);
    auto matrixB = generateRandomMatrix(SIZE, SIZE);
    
    double start_time = omp_get_wtime();
    auto result = multiplyMatricesParallel(matrixA, matrixB);
    double end_time = omp_get_wtime();
    
    cout << "Matrix multiplication completed for " << SIZE << "x" << SIZE << " matrices." << endl;
    cout << "Execution time: " << (end_time - start_time) << " seconds" << endl;
    
    return 0;
}
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
        size_t rowsB = B.size();
        size_t colsB = B[0].size();

        if (colsA != rowsB) {
            throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
        }

        std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));

        #pragma omp parallel for collapse(2) schedule(dynamic)
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
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        for (auto& row : matrix) {
            for (auto& element : row) {
                element = static_cast<double>(std::rand()) / RAND_MAX * 100.0;
            }
        }
    }

    static void displayMatrix(const std::vector<std::vector<double>>& matrix) {
        for (const auto& row : matrix) {
            for (const auto& element : row) {
                std::cout << element << "\t";
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    const size_t N = 512;
    const size_t M = 512;
    const size_t P = 512;

    std::vector<std::vector<double>> matrixA(N, std::vector<double>(M));
    std::vector<std::vector<double>> matrixB(M, std::vector<double>(P));

    ParallelMatrixMultiplier::initializeRandomMatrix(matrixA);
    ParallelMatrixMultiplier::initializeRandomMatrix(matrixB);

    double startTime = omp_get_wtime();
    std::vector<std::vector<double>> result = ParallelMatrixMultiplier::multiply(matrixA, matrixB);
    double endTime = omp_get_wtime();

    std::cout << "Matrix multiplication completed." << std::endl;
    std::cout << "Execution time: " << (endTime - startTime) << " seconds" << std::endl;

    return 0;
}