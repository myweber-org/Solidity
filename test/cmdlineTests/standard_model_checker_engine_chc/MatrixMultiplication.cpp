
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