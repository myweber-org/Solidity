
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

void initializeMatrix(std::vector<std::vector<double>>& matrix, int size) {
    matrix.resize(size, std::vector<double>(size));
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
        }
    }
}

void multiplyMatrices(const std::vector<std::vector<double>>& A,
                      const std::vector<std::vector<double>>& B,
                      std::vector<std::vector<double>>& C,
                      int size) {
    C.resize(size, std::vector<double>(size, 0.0));
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            double sum = 0.0;
            #pragma omp simd reduction(+:sum)
            for (int k = 0; k < size; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

int main() {
    const int MATRIX_SIZE = 512;
    const int NUM_ITERATIONS = 5;
    
    std::vector<std::vector<double>> A, B, C;
    srand(static_cast<unsigned>(time(nullptr)));
    
    std::cout << "Initializing matrices of size " << MATRIX_SIZE << "x" << MATRIX_SIZE << std::endl;
    initializeMatrix(A, MATRIX_SIZE);
    initializeMatrix(B, MATRIX_SIZE);
    
    double total_time = 0.0;
    
    for (int iter = 0; iter < NUM_ITERATIONS; ++iter) {
        double start_time = omp_get_wtime();
        
        multiplyMatrices(A, B, C, MATRIX_SIZE);
        
        double end_time = omp_get_wtime();
        double elapsed = end_time - start_time;
        total_time += elapsed;
        
        std::cout << "Iteration " << iter + 1 << ": " << elapsed << " seconds" << std::endl;
    }
    
    std::cout << "\nAverage multiplication time: " << total_time / NUM_ITERATIONS << " seconds" << std::endl;
    std::cout << "Performance: " << (2.0 * MATRIX_SIZE * MATRIX_SIZE * MATRIX_SIZE) / (total_time / NUM_ITERATIONS) / 1e9 
              << " GFLOP/s" << std::endl;
    
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
    srand(static_cast<unsigned>(time(nullptr)));
    
    const int N = 500;
    cout << "Generating random matrices of size " << N << "x" << N << "..." << endl;
    
    auto matrixA = generateRandomMatrix(N, N);
    auto matrixB = generateRandomMatrix(N, N);
    
    cout << "Performing parallel matrix multiplication..." << endl;
    double startTime = omp_get_wtime();
    
    auto result = multiplyMatricesParallel(matrixA, matrixB);
    
    double endTime = omp_get_wtime();
    cout << "Multiplication completed in " << (endTime - startTime) << " seconds." << endl;
    
    double checksum = 0.0;
    for (int i = 0; i < min(10, N); ++i) {
        for (int j = 0; j < min(10, N); ++j) {
            checksum += result[i][j];
        }
    }
    cout << "Checksum of first 10x10 elements: " << checksum << endl;
    
    return 0;
}