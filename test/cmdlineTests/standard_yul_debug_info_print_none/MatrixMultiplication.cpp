
#include <iostream>
#include <vector>
#include <stdexcept>

class Matrix {
private:
    std::vector<std::vector<double>> data;
    size_t rows;
    size_t cols;

public:
    Matrix(size_t r, size_t c) : rows(r), cols(c) {
        data.resize(rows, std::vector<double>(cols, 0.0));
    }

    void setValue(size_t r, size_t c, double value) {
        if (r >= rows || c >= cols) {
            throw std::out_of_range("Matrix indices out of range");
        }
        data[r][c] = value;
    }

    double getValue(size_t r, size_t c) const {
        if (r >= rows || c >= cols) {
            throw std::out_of_range("Matrix indices out of range");
        }
        return data[r][c];
    }

    size_t getRows() const { return rows; }
    size_t getCols() const { return cols; }

    void print() const {
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                std::cout << data[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }
};

Matrix multiply(const Matrix& a, const Matrix& b) {
    if (a.getCols() != b.getRows()) {
        throw std::invalid_argument("Matrix dimensions mismatch for multiplication");
    }

    Matrix result(a.getRows(), b.getCols());

    for (size_t i = 0; i < a.getRows(); ++i) {
        for (size_t j = 0; j < b.getCols(); ++j) {
            double sum = 0.0;
            for (size_t k = 0; k < a.getCols(); ++k) {
                sum += a.getValue(i, k) * b.getValue(k, j);
            }
            result.setValue(i, j, sum);
        }
    }

    return result;
}

int main() {
    try {
        Matrix matA(2, 3);
        Matrix matB(3, 2);

        matA.setValue(0, 0, 1.0);
        matA.setValue(0, 1, 2.0);
        matA.setValue(0, 2, 3.0);
        matA.setValue(1, 0, 4.0);
        matA.setValue(1, 1, 5.0);
        matA.setValue(1, 2, 6.0);

        matB.setValue(0, 0, 7.0);
        matB.setValue(0, 1, 8.0);
        matB.setValue(1, 0, 9.0);
        matB.setValue(1, 1, 10.0);
        matB.setValue(2, 0, 11.0);
        matB.setValue(2, 1, 12.0);

        std::cout << "Matrix A:" << std::endl;
        matA.print();
        std::cout << "Matrix B:" << std::endl;
        matB.print();

        Matrix matC = multiply(matA, matB);
        std::cout << "Result of multiplication:" << std::endl;
        matC.print();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

using namespace std;

vector<vector<int>> generate_random_matrix(int rows, int cols) {
    vector<vector<int>> matrix(rows, vector<int>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = rand() % 100;
        }
    }
    return matrix;
}

vector<vector<int>> multiply_matrices_parallel(const vector<vector<int>>& A,
                                               const vector<vector<int>>& B) {
    int rows_A = A.size();
    int cols_A = A[0].size();
    int cols_B = B[0].size();
    
    vector<vector<int>> result(rows_A, vector<int>(cols_B, 0));
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rows_A; ++i) {
        for (int j = 0; j < cols_B; ++j) {
            int sum = 0;
            for (int k = 0; k < cols_A; ++k) {
                sum += A[i][k] * B[k][j];
            }
            result[i][j] = sum;
        }
    }
    
    return result;
}

vector<vector<int>> multiply_matrices_sequential(const vector<vector<int>>& A,
                                                 const vector<vector<int>>& B) {
    int rows_A = A.size();
    int cols_A = A[0].size();
    int cols_B = B[0].size();
    
    vector<vector<int>> result(rows_A, vector<int>(cols_B, 0));
    
    for (int i = 0; i < rows_A; ++i) {
        for (int j = 0; j < cols_B; ++j) {
            int sum = 0;
            for (int k = 0; k < cols_A; ++k) {
                sum += A[i][k] * B[k][j];
            }
            result[i][j] = sum;
        }
    }
    
    return result;
}

bool verify_matrices_equal(const vector<vector<int>>& A,
                           const vector<vector<int>>& B) {
    if (A.size() != B.size() || A[0].size() != B[0].size()) {
        return false;
    }
    
    for (size_t i = 0; i < A.size(); ++i) {
        for (size_t j = 0; j < A[0].size(); ++j) {
            if (A[i][j] != B[i][j]) {
                return false;
            }
        }
    }
    
    return true;
}

int main() {
    srand(time(0));
    
    const int N = 500;
    const int M = 500;
    const int P = 500;
    
    cout << "Generating random matrices of size " << N << "x" << M << " and " << M << "x" << P << endl;
    
    auto matrix_A = generate_random_matrix(N, M);
    auto matrix_B = generate_random_matrix(M, P);
    
    cout << "Performing sequential matrix multiplication..." << endl;
    double start_seq = omp_get_wtime();
    auto result_seq = multiply_matrices_sequential(matrix_A, matrix_B);
    double end_seq = omp_get_wtime();
    
    cout << "Performing parallel matrix multiplication..." << endl;
    double start_par = omp_get_wtime();
    auto result_par = multiply_matrices_parallel(matrix_A, matrix_B);
    double end_par = omp_get_wtime();
    
    cout << "\nPerformance Results:" << endl;
    cout << "Sequential execution time: " << (end_seq - start_seq) << " seconds" << endl;
    cout << "Parallel execution time: " << (end_par - start_par) << " seconds" << endl;
    cout << "Speedup: " << (end_seq - start_seq) / (end_par - start_par) << "x" << endl;
    
    if (verify_matrices_equal(result_seq, result_par)) {
        cout << "Verification: Matrices are identical" << endl;
    } else {
        cout << "Verification: Matrices differ!" << endl;
    }
    
    return 0;
}