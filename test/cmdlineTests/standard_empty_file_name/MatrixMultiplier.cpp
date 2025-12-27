
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

bool verifyResults(const vector<vector<double>>& result1,
                   const vector<vector<double>>& result2,
                   double tolerance = 1e-6) {
    if (result1.size() != result2.size() || result1[0].size() != result2[0].size()) {
        return false;
    }
    
    for (size_t i = 0; i < result1.size(); ++i) {
        for (size_t j = 0; j < result1[0].size(); ++j) {
            if (abs(result1[i][j] - result2[i][j]) > tolerance) {
                return false;
            }
        }
    }
    return true;
}

int main() {
    const int SIZE = 500;
    srand(static_cast<unsigned>(time(nullptr)));
    
    cout << "Generating random matrices of size " << SIZE << "x" << SIZE << "..." << endl;
    auto A = generateRandomMatrix(SIZE, SIZE);
    auto B = generateRandomMatrix(SIZE, SIZE);
    
    cout << "Performing sequential matrix multiplication..." << endl;
    auto startSeq = chrono::high_resolution_clock::now();
    auto resultSeq = multiplyMatricesSequential(A, B);
    auto endSeq = chrono::high_resolution_clock::now();
    chrono::duration<double> seqDuration = endSeq - startSeq;
    
    cout << "Performing parallel matrix multiplication..." << endl;
    auto startPar = chrono::high_resolution_clock::now();
    auto resultPar = multiplyMatricesParallel(A, B);
    auto endPar = chrono::high_resolution_clock::now();
    chrono::duration<double> parDuration = endPar - startPar;
    
    cout << "\nPerformance Results:" << endl;
    cout << "Sequential time: " << seqDuration.count() << " seconds" << endl;
    cout << "Parallel time: " << parDuration.count() << " seconds" << endl;
    cout << "Speedup factor: " << seqDuration.count() / parDuration.count() << endl;
    
    cout << "\nVerifying results..." << endl;
    if (verifyResults(resultSeq, resultPar)) {
        cout << "Results match! Parallel computation is correct." << endl;
    } else {
        cout << "Error: Results do not match!" << endl;
    }
    
    return 0;
}