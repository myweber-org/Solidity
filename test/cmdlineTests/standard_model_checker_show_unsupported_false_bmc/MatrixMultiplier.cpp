
#include <iostream>
#include <vector>
#include <chrono>
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

int main() {
    const int SIZE = 500;
    
    auto start = chrono::high_resolution_clock::now();
    vector<vector<double>> matrixA = generateRandomMatrix(SIZE, SIZE);
    vector<vector<double>> matrixB = generateRandomMatrix(SIZE, SIZE);
    auto end = chrono::high_resolution_clock::now();
    
    chrono::duration<double> generationTime = end - start;
    cout << "Matrix generation time: " << generationTime.count() << " seconds" << endl;
    
    start = chrono::high_resolution_clock::now();
    vector<vector<double>> result = multiplyMatrices(matrixA, matrixB);
    end = chrono::high_resolution_clock::now();
    
    chrono::duration<double> multiplicationTime = end - start;
    cout << "Matrix multiplication time: " << multiplicationTime.count() << " seconds" << endl;
    
    double totalOperations = 2.0 * SIZE * SIZE * SIZE;
    double gflops = (totalOperations / multiplicationTime.count()) / 1e9;
    cout << "Performance: " << gflops << " GFLOPS" << endl;
    
    return 0;
}