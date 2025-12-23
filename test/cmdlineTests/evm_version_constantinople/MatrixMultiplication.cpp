
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
    
    const int SIZE = 500;
    cout << "Generating two " << SIZE << "x" << SIZE << " matrices..." << endl;
    
    auto matrixA = generate_random_matrix(SIZE, SIZE);
    auto matrixB = generate_random_matrix(SIZE, SIZE);
    
    cout << "Performing parallel matrix multiplication..." << endl;
    double start_time = omp_get_wtime();
    
    auto result = multiply_matrices_parallel(matrixA, matrixB);
    
    double end_time = omp_get_wtime();
    cout << "Multiplication completed in " << (end_time - start_time) << " seconds." << endl;
    
    cout << "\nVerifying result with a small subset (first 3x3):" << endl;
    for (int i = 0; i < 3 && i < SIZE; ++i) {
        for (int j = 0; j < 3 && j < SIZE; ++j) {
            int sum = 0;
            for (int k = 0; k < SIZE; ++k) {
                sum += matrixA[i][k] * matrixB[k][j];
            }
            cout << sum << "\t";
        }
        cout << endl;
    }
    
    return 0;
}