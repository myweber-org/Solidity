
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
    
    if (n != B.size()) {
        cerr << "Matrix dimensions incompatible for multiplication." << endl;
        return {};
    }
    
    vector<vector<double>> C(m, vector<double>(p, 0.0));
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < p; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
    
    return C;
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    const int SIZE = 500;
    
    cout << "Generating random matrices of size " << SIZE << "x" << SIZE << "..." << endl;
    auto matrixA = generateRandomMatrix(SIZE, SIZE);
    auto matrixB = generateRandomMatrix(SIZE, SIZE);
    
    cout << "Performing parallel matrix multiplication..." << endl;
    double startTime = omp_get_wtime();
    
    auto result = multiplyMatricesParallel(matrixA, matrixB);
    
    double endTime = omp_get_wtime();
    double elapsedTime = endTime - startTime;
    
    cout << "Multiplication completed in " << elapsedTime << " seconds." << endl;
    
    if (!result.empty()) {
        cout << "Result matrix dimensions: " << result.size() << "x" << result[0].size() << endl;
        
        double checksum = 0.0;
        for (int i = 0; i < min(10, SIZE); ++i) {
            for (int j = 0; j < min(10, SIZE); ++j) {
                checksum += result[i][j];
            }
        }
        cout << "Checksum of first 10x10 elements: " << checksum << endl;
    }
    
    return 0;
}