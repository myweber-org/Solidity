
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

using namespace std;

vector<vector<int>> generate_random_matrix(int rows, int cols) {
    vector<vector<int>> matrix(rows, vector<int>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = rand() % 100;
        }
    }
    return matrix;
}

vector<vector<int>> multiply_matrices_parallel(const vector<vector<int>>& A,
                                               const vector<vector<int>>& B) {
    int rows_A = A.size();
    int cols_A = A[0].size();
    int cols_B = B[0].size();
    
    vector<vector<int>> result(rows_A, vector<int>(cols_B, 0));
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rows_A; ++i) {
        for (int j = 0; j < cols_B; ++j) {
            int sum = 0;
            for (int k = 0; k < cols_A; ++k) {
                sum += A[i][k] * B[k][j];
            }
            result[i][j] = sum;
        }
    }
    
    return result;
}

void print_matrix(const vector<vector<int>>& matrix) {
    for (const auto& row : matrix) {
        for (int val : row) {
            cout << val << "\t";
        }
        cout << endl;
    }
}

int main() {
    srand(time(nullptr));
    
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
    
    if (N <= 10 && P <= 10) {
        cout << "\nFirst matrix (A):" << endl;
        print_matrix(matrix_A);
        
        cout << "\nSecond matrix (B):" << endl;
        print_matrix(matrix_B);
        
        cout << "\nResult matrix (A x B):" << endl;
        print_matrix(result);
    } else {
        cout << "Matrices are too large to display. Showing corner elements:" << endl;
        cout << "Result[0][0] = " << result[0][0] << endl;
        cout << "Result[0][" << P-1 << "] = " << result[0][P-1] << endl;
        cout << "Result[" << N-1 << "][0] = " << result[N-1][0] << endl;
        cout << "Result[" << N-1 << "][" << P-1 << "] = " << result[N-1][P-1] << endl;
    }
    
    return 0;
}