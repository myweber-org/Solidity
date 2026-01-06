
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
    int m = A.size();
    int n = A[0].size();
    int p = B[0].size();
    
    if (n != B.size()) {
        cerr << "Matrix dimensions incompatible for multiplication." << endl;
        exit(1);
    }
    
    vector<vector<double>> C(m, vector<double>(p, 0.0));
    
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

void print_matrix_summary(const vector<vector<double>>& matrix, const string& name) {
    double sum = 0.0;
    for (const auto& row : matrix) {
        for (double val : row) {
            sum += val;
        }
    }
    cout << name << " total sum: " << sum << endl;
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    const int SIZE = 500;
    
    cout << "Generating random matrices of size " << SIZE << "x" << SIZE << "..." << endl;
    auto matrix_A = generate_random_matrix(SIZE, SIZE);
    auto matrix_B = generate_random_matrix(SIZE, SIZE);
    
    cout << "Performing parallel matrix multiplication..." << endl;
    double start_time = omp_get_wtime();
    
    auto result_matrix = multiply_matrices_parallel(matrix_A, matrix_B);
    
    double end_time = omp_get_wtime();
    
    cout << "Multiplication completed in " << (end_time - start_time) << " seconds." << endl;
    
    print_matrix_summary(result_matrix, "Result matrix");
    
    return 0;
}