
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

vector<vector<double>> multiplyMatricesSequential(const vector<vector<double>>& A,
                                                  const vector<vector<double>>& B) {
    int m = A.size();
    int n = A[0].size();
    int p = B[0].size();
    
    vector<vector<double>> C(m, vector<double>(p, 0.0));
    
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

bool compareMatrices(const vector<vector<double>>& A, const vector<vector<double>>& B, double tolerance = 1e-10) {
    if (A.size() != B.size() || A[0].size() != B[0].size()) {
        return false;
    }
    
    for (size_t i = 0; i < A.size(); ++i) {
        for (size_t j = 0; j < A[0].size(); ++j) {
            if (abs(A[i][j] - B[i][j]) > tolerance) {
                return false;
            }
        }
    }
    return true;
}

int main() {
    srand(static_cast<unsigned>(time(0)));
    
    const int SIZE = 500;
    
    cout << "Generating random matrices of size " << SIZE << "x" << SIZE << "..." << endl;
    auto A = generateRandomMatrix(SIZE, SIZE);
    auto B = generateRandomMatrix(SIZE, SIZE);
    
    cout << "Performing sequential matrix multiplication..." << endl;
    double start_seq = omp_get_wtime();
    auto C_seq = multiplyMatricesSequential(A, B);
    double end_seq = omp_get_wtime();
    
    cout << "Performing parallel matrix multiplication..." << endl;
    double start_par = omp_get_wtime();
    auto C_par = multiplyMatricesParallel(A, B);
    double end_par = omp_get_wtime();
    
    cout << "\nResults:" << endl;
    cout << "Sequential execution time: " << (end_seq - start_seq) << " seconds" << endl;
    cout << "Parallel execution time: " << (end_par - start_par) << " seconds" << endl;
    cout << "Speedup: " << (end_seq - start_seq) / (end_par - start_par) << "x" << endl;
    
    if (compareMatrices(C_seq, C_par)) {
        cout << "Matrix multiplication results are identical." << endl;
    } else {
        cout << "ERROR: Matrix multiplication results differ!" << endl;
    }
    
    return 0;
}