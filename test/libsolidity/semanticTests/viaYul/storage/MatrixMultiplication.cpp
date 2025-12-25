
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
    int m = A.size();
    int n = A[0].size();
    int p = B[0].size();
    
    if (n != B.size()) {
        cerr << "Error: Matrix dimensions incompatible for multiplication." << endl;
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

void printMatrix(const vector<vector<double>>& matrix, int maxRows = 5, int maxCols = 5) {
    int rows = min(static_cast<int>(matrix.size()), maxRows);
    int cols = min(static_cast<int>(matrix[0].size()), maxCols);
    
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            cout << matrix[i][j] << "\t";
        }
        if (cols < matrix[0].size()) cout << "...";
        cout << endl;
    }
    if (rows < matrix.size()) cout << "...\n";
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    const int M = 500;
    const int N = 500;
    const int P = 500;
    
    cout << "Generating random matrices of size " << M << "x" << N << " and " << N << "x" << P << "..." << endl;
    
    auto matrixA = generateRandomMatrix(M, N);
    auto matrixB = generateRandomMatrix(N, P);
    
    cout << "Performing parallel matrix multiplication..." << endl;
    
    double startTime = omp_get_wtime();
    auto result = multiplyMatricesParallel(matrixA, matrixB);
    double endTime = omp_get_wtime();
    
    cout << "Matrix multiplication completed in " << (endTime - startTime) << " seconds." << endl;
    
    cout << "\nFirst 5x5 elements of result matrix:" << endl;
    printMatrix(result);
    
    return 0;
}