
#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <omp.h>

using namespace std;
using namespace std::chrono;

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

vector<vector<double>> multiplyMatricesSequential(const vector<vector<double>>& A,
                                                  const vector<vector<double>>& B) {
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();
    
    vector<vector<double>> result(rowsA, vector<double>(colsB, 0.0));
    
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

bool verifyResults(const vector<vector<double>>& mat1,
                   const vector<vector<double>>& mat2,
                   double tolerance = 1e-6) {
    if (mat1.size() != mat2.size() || mat1[0].size() != mat2[0].size()) {
        return false;
    }
    
    for (size_t i = 0; i < mat1.size(); ++i) {
        for (size_t j = 0; j < mat1[0].size(); ++j) {
            if (abs(mat1[i][j] - mat2[i][j]) > tolerance) {
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
    auto matrixA = generateRandomMatrix(ROWS_A, COLS_A);
    auto matrixB = generateRandomMatrix(COLS_A, COLS_B);
    
    cout << "Performing sequential multiplication..." << endl;
    auto startSeq = high_resolution_clock::now();
    auto resultSeq = multiplyMatricesSequential(matrixA, matrixB);
    auto endSeq = high_resolution_clock::now();
    auto durationSeq = duration_cast<milliseconds>(endSeq - startSeq);
    
    cout << "Performing parallel multiplication..." << endl;
    auto startPar = high_resolution_clock::now();
    auto resultPar = multiplyMatricesParallel(matrixA, matrixB);
    auto endPar = high_resolution_clock::now();
    auto durationPar = duration_cast<milliseconds>(endPar - startPar);
    
    cout << "\nPerformance Results:" << endl;
    cout << "Sequential time: " << durationSeq.count() << " ms" << endl;
    cout << "Parallel time: " << durationPar.count() << " ms" << endl;
    cout << "Speedup: " << static_cast<double>(durationSeq.count()) / durationPar.count() << "x" << endl;
    
    if (verifyResults(resultSeq, resultPar)) {
        cout << "Results verification: PASSED" << endl;
    } else {
        cout << "Results verification: FAILED" << endl;
    }
    
    return 0;
}