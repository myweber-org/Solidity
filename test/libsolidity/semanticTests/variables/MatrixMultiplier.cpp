
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

vector<vector<double>> multiplyMatricesParallel(const vector<vector<double>>& A,
                                                const vector<vector<double>>& B) {
    int m = A.size();
    int n = A[0].size();
    int p = B[0].size();
    
    vector<vector<double>> result(m, vector<double>(p, 0.0));
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < p; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k) {
                sum += A[i][k] * B[k][j];
            }
            result[i][j] = sum;
        }
    }
    
    return result;
}

vector<vector<double>> multiplyMatricesSequential(const vector<vector<double>>& A,
                                                  const vector<vector<double>>& B) {
    int m = A.size();
    int n = A[0].size();
    int p = B[0].size();
    
    vector<vector<double>> result(m, vector<double>(p, 0.0));
    
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < p; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k) {
                sum += A[i][k] * B[k][j];
            }
            result[i][j] = sum;
        }
    }
    
    return result;
}

bool verifyResults(const vector<vector<double>>& mat1,
                   const vector<vector<double>>& mat2,
                   double tolerance = 1e-10) {
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
    srand(static_cast<unsigned>(time(nullptr)));
    
    const int SIZE = 500;
    
    cout << "Generating random matrices of size " << SIZE << "x" << SIZE << "..." << endl;
    auto matrixA = generateRandomMatrix(SIZE, SIZE);
    auto matrixB = generateRandomMatrix(SIZE, SIZE);
    
    cout << "Performing parallel matrix multiplication..." << endl;
    double startParallel = omp_get_wtime();
    auto resultParallel = multiplyMatricesParallel(matrixA, matrixB);
    double endParallel = omp_get_wtime();
    
    cout << "Performing sequential matrix multiplication..." << endl;
    double startSequential = omp_get_wtime();
    auto resultSequential = multiplyMatricesSequential(matrixA, matrixB);
    double endSequential = omp_get_wtime();
    
    cout << "\nPerformance Results:" << endl;
    cout << "Parallel execution time: " << (endParallel - startParallel) << " seconds" << endl;
    cout << "Sequential execution time: " << (endSequential - startSequential) << " seconds" << endl;
    cout << "Speedup factor: " << (endSequential - startSequential) / (endParallel - startParallel) << endl;
    
    cout << "\nVerifying results..." << endl;
    if (verifyResults(resultParallel, resultSequential)) {
        cout << "Results match! Parallel computation is correct." << endl;
    } else {
        cout << "Error: Parallel and sequential results do not match!" << endl;
    }
    
    return 0;
}