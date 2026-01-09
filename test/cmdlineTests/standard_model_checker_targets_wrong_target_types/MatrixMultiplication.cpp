
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

vector<vector<double>> multiply_matrices_sequential(const vector<vector<double>>& A,
                                                    const vector<vector<double>>& B) {
    int rows_A = A.size();
    int cols_A = A[0].size();
    int cols_B = B[0].size();
    
    vector<vector<double>> result(rows_A, vector<double>(cols_B, 0.0));
    
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

bool verify_matrices(const vector<vector<double>>& mat1, const vector<vector<double>>& mat2, double tolerance = 1e-6) {
    if (mat1.size() != mat2.size() || mat1[0].size() != mat2[0].size()) {
        return false;
    }
    
    for (size_t i = 0; i < mat1.size(); ++i) {
        for (size_t j = 0; j < mat1[0].size(); ++j) {
            if (abs(mat1[i][j] - mat2[i][j]) > tolerance) {
                return false;
            }
        }
    }
    return true;
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
    double start_parallel = omp_get_wtime();
    auto result_parallel = multiply_matrices_parallel(matrix_A, matrix_B);
    double end_parallel = omp_get_wtime();
    
    cout << "Performing sequential matrix multiplication..." << endl;
    double start_sequential = omp_get_wtime();
    auto result_sequential = multiply_matrices_sequential(matrix_A, matrix_B);
    double end_sequential = omp_get_wtime();
    
    cout << "\nPerformance Results:" << endl;
    cout << "Parallel execution time: " << (end_parallel - start_parallel) << " seconds" << endl;
    cout << "Sequential execution time: " << (end_sequential - start_sequential) << " seconds" << endl;
    cout << "Speedup factor: " << (end_sequential - start_sequential) / (end_parallel - start_parallel) << endl;
    
    cout << "\nVerifying results..." << endl;
    if (verify_matrices(result_parallel, result_sequential)) {
        cout << "Verification passed: Parallel and sequential results match." << endl;
    } else {
        cout << "Verification failed: Results do not match!" << endl;
    }
    
    return 0;
}