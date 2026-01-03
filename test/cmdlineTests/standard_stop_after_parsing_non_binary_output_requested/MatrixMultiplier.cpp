
#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <omp.h>

using namespace std;
using namespace std::chrono;

vector<vector<int>> generateRandomMatrix(int rows, int cols) {
    vector<vector<int>> matrix(rows, vector<int>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = rand() % 100;
        }
    }
    return matrix;
}

vector<vector<int>> multiplyMatricesSequential(const vector<vector<int>>& A,
                                               const vector<vector<int>>& B) {
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();
    
    vector<vector<int>> result(rowsA, vector<int>(colsB, 0));
    
    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            for (int k = 0; k < colsA; ++k) {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    
    return result;
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

bool verifyResults(const vector<vector<int>>& seqResult,
                   const vector<vector<int>>& parResult) {
    if (seqResult.size() != parResult.size()) return false;
    if (seqResult[0].size() != parResult[0].size()) return false;
    
    for (size_t i = 0; i < seqResult.size(); ++i) {
        for (size_t j = 0; j < seqResult[0].size(); ++j) {
            if (seqResult[i][j] != parResult[i][j]) {
                return false;
            }
        }
    }
    return true;
}

int main() {
    const int ROWS_A = 500;
    const int COLS_A = 500;
    const int COLS_B = 500;
    
    srand(42);
    
    cout << "Generating random matrices..." << endl;
    auto A = generateRandomMatrix(ROWS_A, COLS_A);
    auto B = generateRandomMatrix(COLS_A, COLS_B);
    
    cout << "Performing sequential multiplication..." << endl;
    auto start = high_resolution_clock::now();
    auto seqResult = multiplyMatricesSequential(A, B);
    auto stop = high_resolution_clock::now();
    auto seqDuration = duration_cast<milliseconds>(stop - start);
    
    cout << "Performing parallel multiplication..." << endl;
    start = high_resolution_clock::now();
    auto parResult = multiplyMatricesParallel(A, B);
    stop = high_resolution_clock::now();
    auto parDuration = duration_cast<milliseconds>(stop - start);
    
    cout << "\nPerformance Results:" << endl;
    cout << "Sequential time: " << seqDuration.count() << " ms" << endl;
    cout << "Parallel time: " << parDuration.count() << " ms" << endl;
    cout << "Speedup: " << (double)seqDuration.count() / parDuration.count() << "x" << endl;
    
    if (verifyResults(seqResult, parResult)) {
        cout << "Results verification: PASSED" << endl;
    } else {
        cout << "Results verification: FAILED" << endl;
    }
    
    return 0;
}