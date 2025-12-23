
#include <iostream>
#include <vector>
#include <chrono>
#include <omp.h>

using namespace std;
using namespace chrono;

vector<vector<double>> generateRandomMatrix(int rows, int cols) {
    vector<vector<double>> matrix(rows, vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
        }
    }
    return matrix;
}

vector<vector<double>> multiplyMatricesSequential(const vector<vector<double>>& A,
                                                  const vector<vector<double>>& B) {
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();
    
    vector<vector<double>> result(rowsA, vector<double>(colsB, 0.0));
    
    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            for (int k = 0; k < colsA; ++k) {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    
    return result;
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

bool verifyResults(const vector<vector<double>>& seqResult,
                   const vector<vector<double>>& parResult,
                   double tolerance = 1e-10) {
    if (seqResult.size() != parResult.size() || seqResult[0].size() != parResult[0].size()) {
        return false;
    }
    
    for (size_t i = 0; i < seqResult.size(); ++i) {
        for (size_t j = 0; j < seqResult[0].size(); ++j) {
            if (abs(seqResult[i][j] - parResult[i][j]) > tolerance) {
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
    auto seqResult = multiplyMatricesSequential(matrixA, matrixB);
    auto endSeq = high_resolution_clock::now();
    auto durationSeq = duration_cast<milliseconds>(endSeq - startSeq);
    
    cout << "Performing parallel multiplication..." << endl;
    auto startPar = high_resolution_clock::now();
    auto parResult = multiplyMatricesParallel(matrixA, matrixB);
    auto endPar = high_resolution_clock::now();
    auto durationPar = duration_cast<milliseconds>(endPar - startPar);
    
    cout << "\nPerformance Results:" << endl;
    cout << "Sequential time: " << durationSeq.count() << " ms" << endl;
    cout << "Parallel time: " << durationPar.count() << " ms" << endl;
    cout << "Speedup: " << static_cast<double>(durationSeq.count()) / durationPar.count() << "x" << endl;
    
    cout << "\nVerifying results..." << endl;
    if (verifyResults(seqResult, parResult)) {
        cout << "Results match!" << endl;
    } else {
        cout << "ERROR: Results do not match!" << endl;
    }
    
    return 0;
}