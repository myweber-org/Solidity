
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

bool verifyResults(const vector<vector<int>>& result1,
                   const vector<vector<int>>& result2) {
    if (result1.size() != result2.size() || result1[0].size() != result2[0].size()) {
        return false;
    }
    
    for (size_t i = 0; i < result1.size(); ++i) {
        for (size_t j = 0; j < result1[0].size(); ++j) {
            if (result1[i][j] != result2[i][j]) {
                return false;
            }
        }
    }
    return true;
}

int main() {
    srand(time(nullptr));
    
    const int N = 500;
    const int M = 500;
    const int P = 500;
    
    cout << "Generating random matrices of size " << N << "x" << M << " and " << M << "x" << P << endl;
    
    auto matrixA = generateRandomMatrix(N, M);
    auto matrixB = generateRandomMatrix(M, P);
    
    cout << "Starting parallel multiplication..." << endl;
    double startParallel = omp_get_wtime();
    auto resultParallel = multiplyMatricesParallel(matrixA, matrixB);
    double endParallel = omp_get_wtime();
    
    cout << "Starting sequential multiplication..." << endl;
    double startSequential = omp_get_wtime();
    auto resultSequential = multiplyMatricesSequential(matrixA, matrixB);
    double endSequential = omp_get_wtime();
    
    cout << "\nPerformance Results:" << endl;
    cout << "Parallel execution time: " << (endParallel - startParallel) << " seconds" << endl;
    cout << "Sequential execution time: " << (endSequential - startSequential) << " seconds" << endl;
    cout << "Speedup: " << (endSequential - startSequential) / (endParallel - startParallel) << "x" << endl;
    
    if (verifyResults(resultParallel, resultSequential)) {
        cout << "Results verification: PASSED" << endl;
    } else {
        cout << "Results verification: FAILED" << endl;
    }
    
    return 0;
}