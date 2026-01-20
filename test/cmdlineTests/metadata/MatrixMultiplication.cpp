
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

vector<vector<int>> multiplyMatricesSequential(const vector<vector<int>>& A,
                                              const vector<vector<int>>& B) {
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();
    
    vector<vector<int>> result(rowsA, vector<int>(colsB, 0));
    
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

bool verifyMatricesEqual(const vector<vector<int>>& A,
                        const vector<vector<int>>& B) {
    if (A.size() != B.size() || A[0].size() != B[0].size()) {
        return false;
    }
    
    for (size_t i = 0; i < A.size(); ++i) {
        for (size_t j = 0; j < A[0].size(); ++j) {
            if (A[i][j] != B[i][j]) {
                return false;
            }
        }
    }
    return true;
}

int main() {
    srand(time(0));
    
    const int SIZE = 500;
    cout << "Generating " << SIZE << "x" << SIZE << " matrices..." << endl;
    
    auto matrixA = generateRandomMatrix(SIZE, SIZE);
    auto matrixB = generateRandomMatrix(SIZE, SIZE);
    
    cout << "Performing sequential multiplication..." << endl;
    double startSeq = omp_get_wtime();
    auto resultSeq = multiplyMatricesSequential(matrixA, matrixB);
    double endSeq = omp_get_wtime();
    
    cout << "Performing parallel multiplication..." << endl;
    double startPar = omp_get_wtime();
    auto resultPar = multiplyMatricesParallel(matrixA, matrixB);
    double endPar = omp_get_wtime();
    
    cout << "\nResults verification: ";
    if (verifyMatricesEqual(resultSeq, resultPar)) {
        cout << "PASSED" << endl;
    } else {
        cout << "FAILED" << endl;
        return 1;
    }
    
    cout << "\nPerformance metrics:" << endl;
    cout << "Sequential time: " << (endSeq - startSeq) << " seconds" << endl;
    cout << "Parallel time: " << (endPar - startPar) << " seconds" << endl;
    cout << "Speedup: " << (endSeq - startSeq) / (endPar - startPar) << "x" << endl;
    
    return 0;
}