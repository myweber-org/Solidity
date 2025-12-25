
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

void printMatrixStats(const vector<vector<double>>& matrix) {
    double minVal = matrix[0][0];
    double maxVal = matrix[0][0];
    double sum = 0.0;
    int totalElements = 0;
    
    for (const auto& row : matrix) {
        for (double val : row) {
            if (val < minVal) minVal = val;
            if (val > maxVal) maxVal = val;
            sum += val;
            totalElements++;
        }
    }
    
    cout << "Matrix Statistics:" << endl;
    cout << "  Dimensions: " << matrix.size() << "x" << matrix[0].size() << endl;
    cout << "  Min value: " << minVal << endl;
    cout << "  Max value: " << maxVal << endl;
    cout << "  Average: " << sum / totalElements << endl;
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    const int N = 500;
    const int M = 500;
    const int P = 500;
    
    cout << "Generating random matrices..." << endl;
    auto matrixA = generateRandomMatrix(N, M);
    auto matrixB = generateRandomMatrix(M, P);
    
    cout << "Multiplying matrices using OpenMP..." << endl;
    double startTime = omp_get_wtime();
    
    auto result = multiplyMatrices(matrixA, matrixB);
    
    double endTime = omp_get_wtime();
    double executionTime = endTime - startTime;
    
    cout << "Matrix multiplication completed!" << endl;
    cout << "Execution time: " << executionTime << " seconds" << endl;
    
    printMatrixStats(result);
    
    return 0;
}