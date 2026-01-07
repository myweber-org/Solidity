
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

std::vector<std::vector<double>> generate_random_matrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
        }
    }
    return matrix;
}

std::vector<std::vector<double>> multiply_matrices_parallel(
    const std::vector<std::vector<double>>& A,
    const std::vector<std::vector<double>>& B) {

    int m = A.size();
    int n = A[0].size();
    int p = B[0].size();

    std::vector<std::vector<double>> C(m, std::vector<double>(p, 0.0));

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
    auto matrixA = generate_random_matrix(SIZE, SIZE);
    auto matrixB = generate_random_matrix(SIZE, SIZE);

    double start_time = omp_get_wtime();
    auto result = multiply_matrices_parallel(matrixA, matrixB);
    double end_time = omp_get_wtime();

    std::cout << "Parallel matrix multiplication completed." << std::endl;
    std::cout << "Matrix size: " << SIZE << "x" << SIZE << std::endl;
    std::cout << "Execution time: " << end_time - start_time << " seconds" << std::endl;

    return 0;
}
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
        exit(1);
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
    
    auto matrixA = generateRandomMatrix(SIZE, SIZE);
    auto matrixB = generateRandomMatrix(SIZE, SIZE);
    
    double startTime = omp_get_wtime();
    auto result = multiplyMatricesParallel(matrixA, matrixB);
    double endTime = omp_get_wtime();
    
    cout << "Matrix multiplication completed for " << SIZE << "x" << SIZE << " matrices." << endl;
    cout << "Execution time: " << (endTime - startTime) << " seconds" << endl;
    
    return 0;
}