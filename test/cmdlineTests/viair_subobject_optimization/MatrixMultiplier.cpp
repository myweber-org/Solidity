
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

vector<vector<int>> multiplyMatrices(const vector<vector<int>>& A,
                                     const vector<vector<int>>& B) {
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();
    
    vector<vector<int>> result(rowsA, vector<int>(colsB, 0));
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            int sum = 0;
            for (int k = 0; k < colsA; ++k) {
                sum += A[i][k] * B[k][j];
            }
            result[i][j] = sum;
        }
    }
    
    return result;
}

void printMatrix(const vector<vector<int>>& matrix) {
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
    
    cout << "Generating matrices..." << endl;
    vector<vector<int>> A = generateRandomMatrix(N, M);
    vector<vector<int>> B = generateRandomMatrix(M, P);
    
    cout << "Multiplying matrices of size " << N << "x" << M 
         << " and " << M << "x" << P << "..." << endl;
    
    double start_time = omp_get_wtime();
    vector<vector<int>> C = multiplyMatrices(A, B);
    double end_time = omp_get_wtime();
    
    cout << "Multiplication completed in " << (end_time - start_time) 
         << " seconds" << endl;
    
    cout << "Sample of result matrix (first 3x3 elements):" << endl;
    for (int i = 0; i < min(3, N); ++i) {
        for (int j = 0; j < min(3, P); ++j) {
            cout << C[i][j] << "\t";
        }
        cout << endl;
    }
    
    return 0;
}