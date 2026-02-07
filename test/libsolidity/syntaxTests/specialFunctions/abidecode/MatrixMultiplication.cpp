
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

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    const int N = 500;
    
    auto matrixA = generateRandomMatrix(N, N);
    auto matrixB = generateRandomMatrix(N, N);
    
    double startTime = omp_get_wtime();
    auto result = multiplyMatrices(matrixA, matrixB);
    double endTime = omp_get_wtime();
    
    cout << "Matrix multiplication completed for " << N << "x" << N << " matrices." << endl;
    cout << "Execution time: " << (endTime - startTime) << " seconds" << endl;
    
    return 0;
}
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

bool verifyMatricesEqual(const vector<vector<int>>& mat1,
                         const vector<vector<int>>& mat2) {
    if (mat1.size() != mat2.size() || mat1[0].size() != mat2[0].size()) {
        return false;
    }
    
    for (size_t i = 0; i < mat1.size(); ++i) {
        for (size_t j = 0; j < mat1[0].size(); ++j) {
            if (mat1[i][j] != mat2[i][j]) {
                return false;
            }
        }
    }
    return true;
}

int main() {
    srand(time(nullptr));
    
    const int SIZE = 500;
    
    cout << "Generating random matrices of size " << SIZE << "x" << SIZE << "..." << endl;
    auto matrixA = generateRandomMatrix(SIZE, SIZE);
    auto matrixB = generateRandomMatrix(SIZE, SIZE);
    
    cout << "Performing sequential matrix multiplication..." << endl;
    double startTime = omp_get_wtime();
    auto sequentialResult = multiplyMatricesSequential(matrixA, matrixB);
    double sequentialTime = omp_get_wtime() - startTime;
    
    cout << "Performing parallel matrix multiplication..." << endl;
    startTime = omp_get_wtime();
    auto parallelResult = multiplyMatricesParallel(matrixA, matrixB);
    double parallelTime = omp_get_wtime() - startTime;
    
    cout << "\nPerformance Results:" << endl;
    cout << "Sequential time: " << sequentialTime << " seconds" << endl;
    cout << "Parallel time: " << parallelTime << " seconds" << endl;
    cout << "Speedup: " << sequentialTime / parallelTime << "x" << endl;
    
    bool resultsMatch = verifyMatricesEqual(sequentialResult, parallelResult);
    cout << "\nVerification: " << (resultsMatch ? "PASSED" : "FAILED") << endl;
    
    return 0;
}