
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

std::vector<std::vector<double>> generate_random_matrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
        }
    }
    return matrix;
}

std::vector<std::vector<double>> multiply_matrices_parallel(
    const std::vector<std::vector<double>>& A,
    const std::vector<std::vector<double>>& B) {
    
    int m = A.size();
    int n = A[0].size();
    int p = B[0].size();
    
    std::vector<std::vector<double>> C(m, std::vector<double>(p, 0.0));
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < p; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
    
    return C;
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    const int SIZE = 500;
    
    std::cout << "Generating random matrices of size " << SIZE << "x" << SIZE << std::endl;
    auto A = generate_random_matrix(SIZE, SIZE);
    auto B = generate_random_matrix(SIZE, SIZE);
    
    std::cout << "Performing parallel matrix multiplication..." << std::endl;
    double start_time = omp_get_wtime();
    
    auto C = multiply_matrices_parallel(A, B);
    
    double end_time = omp_get_wtime();
    double elapsed = end_time - start_time;
    
    std::cout << "Multiplication completed in " << elapsed << " seconds" << std::endl;
    
    double sample_sum = 0.0;
    for (int i = 0; i < 10; ++i) {
        sample_sum += C[i][i];
    }
    std::cout << "Sample diagonal sum (first 10 elements): " << sample_sum << std::endl;
    
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

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    const int N = 500;
    
    auto matrixA = generateRandomMatrix(N, N);
    auto matrixB = generateRandomMatrix(N, N);
    
    double startTime = omp_get_wtime();
    auto result = multiplyMatrices(matrixA, matrixB);
    double endTime = omp_get_wtime();
    
    cout << "Matrix multiplication completed for " << N << "x" << N << " matrices." << endl;
    cout << "Execution time: " << (endTime - startTime) << " seconds" << endl;
    
    double checksum = 0.0;
    for (int i = 0; i < min(10, N); ++i) {
        for (int j = 0; j < min(10, N); ++j) {
            checksum += result[i][j];
        }
    }
    cout << "Checksum of first 10x10 elements: " << checksum << endl;
    
    return 0;
}