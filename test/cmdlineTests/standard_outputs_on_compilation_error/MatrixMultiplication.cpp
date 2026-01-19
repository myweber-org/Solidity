
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

using namespace std;

vector<vector<double>> generate_random_matrix(int rows, int cols) {
    vector<vector<double>> matrix(rows, vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
        }
    }
    return matrix;
}

vector<vector<double>> multiply_matrices_parallel(const vector<vector<double>>& A,
                                                  const vector<vector<double>>& B) {
    int rows_A = A.size();
    int cols_A = A[0].size();
    int cols_B = B[0].size();
    
    vector<vector<double>> result(rows_A, vector<double>(cols_B, 0.0));
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rows_A; ++i) {
        for (int j = 0; j < cols_B; ++j) {
            double sum = 0.0;
            for (int k = 0; k < cols_A; ++k) {
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
    const int M = 500;
    const int P = 500;
    
    cout << "Generating random matrices of size " << N << "x" << M << " and " << M << "x" << P << "..." << endl;
    auto matrix_A = generate_random_matrix(N, M);
    auto matrix_B = generate_random_matrix(M, P);
    
    cout << "Performing parallel matrix multiplication..." << endl;
    double start_time = omp_get_wtime();
    auto result = multiply_matrices_parallel(matrix_A, matrix_B);
    double end_time = omp_get_wtime();
    
    cout << "Multiplication completed in " << (end_time - start_time) << " seconds." << endl;
    
    return 0;
}