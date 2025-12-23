
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
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX * 100.0;
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
        cerr << "Matrix dimensions incompatible for multiplication." << endl;
        return {};
    }
    
    vector<vector<double>> result(m, vector<double>(p, 0.0));
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < p; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k) {
                sum += A[i][k] * B[k][j];
            }
            result[i][j] = sum;
        }
    }
    
    return result;
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
    
    const int SIZE = 1000;
    cout << "Generating random matrices of size " << SIZE << "x" << SIZE << "..." << endl;
    
    auto matrixA = generateRandomMatrix(SIZE, SIZE);
    auto matrixB = generateRandomMatrix(SIZE, SIZE);
    
    cout << "Starting parallel matrix multiplication..." << endl;
    double startTime = omp_get_wtime();
    
    auto result = multiplyMatricesParallel(matrixA, matrixB);
    
    double endTime = omp_get_wtime();
    double elapsedTime = endTime - startTime;
    
    cout << "Parallel multiplication completed in " << elapsedTime << " seconds." << endl;
    
    if (SIZE <= 10) {
        cout << "\nMatrix A (first " << min(SIZE, 5) << "x" << min(SIZE, 5) << "):" << endl;
        printMatrix(matrixA);
        
        cout << "\nMatrix B (first " << min(SIZE, 5) << "x" << min(SIZE, 5) << "):" << endl;
        printMatrix(matrixB);
        
        cout << "\nResult (first " << min(SIZE, 5) << "x" << min(SIZE, 5) << "):" << endl;
        printMatrix(result);
    }
    
    cout << "\nPerformance: " << (2.0 * SIZE * SIZE * SIZE) / (elapsedTime * 1e9) << " GFLOPS" << endl;
    
    return 0;
}