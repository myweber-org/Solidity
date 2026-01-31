
#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
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

vector<vector<double>> multiplyMatrices(const vector<vector<double>>& A,
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

void printMatrixStats(const vector<vector<double>>& matrix, const string& name) {
    double sum = 0.0;
    double minVal = matrix[0][0];
    double maxVal = matrix[0][0];
    
    for (const auto& row : matrix) {
        for (double val : row) {
            sum += val;
            if (val < minVal) minVal = val;
            if (val > maxVal) maxVal = val;
        }
    }
    
    cout << name << " - Size: " << matrix.size() << "x" << matrix[0].size()
         << ", Sum: " << sum
         << ", Min: " << minVal
         << ", Max: " << maxVal << endl;
}

int main() {
    const int SIZE = 500;
    srand(42);
    
    auto start = chrono::high_resolution_clock::now();
    auto matrixA = generateRandomMatrix(SIZE, SIZE);
    auto matrixB = generateRandomMatrix(SIZE, SIZE);
    auto genTime = chrono::high_resolution_clock::now() - start;
    
    cout << "Matrix generation time: "
         << chrono::duration<double>(genTime).count() << " seconds" << endl;
    
    start = chrono::high_resolution_clock::now();
    auto result = multiplyMatrices(matrixA, matrixB);
    auto multTime = chrono::high_resolution_clock::now() - start;
    
    cout << "Matrix multiplication time: "
         << chrono::duration<double>(multTime).count() << " seconds" << endl;
    
    printMatrixStats(matrixA, "Matrix A");
    printMatrixStats(matrixB, "Matrix B");
    printMatrixStats(result, "Result Matrix");
    
    return 0;
}