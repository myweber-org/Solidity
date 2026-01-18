
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

    void setValue(size_t i, size_t j, double value) {
        if (i >= rows || j >= cols) {
            throw std::out_of_range("Matrix indices out of range");
        }
        data[i][j] = value;
    }

    double getValue(size_t i, size_t j) const {
        if (i >= rows || j >= cols) {
            throw std::out_of_range("Matrix indices out of range");
        }
        return data[i][j];
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

    size_t m = a.getRows();
    size_t n = a.getCols();
    size_t p = b.getCols();

    Matrix result(m, p);

    for (size_t i = 0; i < m; ++i) {
        for (size_t j = 0; j < p; ++j) {
            double sum = 0.0;
            for (size_t k = 0; k < n; ++k) {
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
        matA.setValue(0, 0, 1.0);
        matA.setValue(0, 1, 2.0);
        matA.setValue(0, 2, 3.0);
        matA.setValue(1, 0, 4.0);
        matA.setValue(1, 1, 5.0);
        matA.setValue(1, 2, 6.0);

        Matrix matB(3, 2);
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
        std::cout << "Result of A * B:" << std::endl;
        matC.print();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
#include <iostream>
#include <vector>

std::vector<std::vector<int>> multiplyMatrices(const std::vector<std::vector<int>>& matrixA,
                                                const std::vector<std::vector<int>>& matrixB) {
    int rowsA = matrixA.size();
    int colsA = matrixA[0].size();
    int rowsB = matrixB.size();
    int colsB = matrixB[0].size();

    if (colsA != rowsB) {
        throw std::invalid_argument("Matrix dimensions are not compatible for multiplication.");
    }

    std::vector<std::vector<int>> result(rowsA, std::vector<int>(colsB, 0));

    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            for (int k = 0; k < colsA; ++k) {
                result[i][j] += matrixA[i][k] * matrixB[k][j];
            }
        }
    }

    return result;
}

void printMatrix(const std::vector<std::vector<int>>& matrix) {
    for (const auto& row : matrix) {
        for (int val : row) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    std::vector<std::vector<int>> A = {{1, 2, 3},
                                        {4, 5, 6}};

    std::vector<std::vector<int>> B = {{7, 8},
                                        {9, 10},
                                        {11, 12}};

    try {
        std::vector<std::vector<int>> C = multiplyMatrices(A, B);
        std::cout << "Resultant matrix:" << std::endl;
        printMatrix(C);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
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
    
    int rows_A = A.size();
    int cols_A = A[0].size();
    int cols_B = B[0].size();
    
    std::vector<std::vector<double>> result(rows_A, std::vector<double>(cols_B, 0.0));
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rows_A; ++i) {
        for (int j = 0; j < cols_B; ++j) {
            double sum = 0.0;
            for (int k = 0; k < cols_A; ++k) {
                sum += A[i][k] * B[k][j];
            }
            result[i][j] = sum;
        }
    }
    
    return result;
}

void print_matrix_dimensions(const std::vector<std::vector<double>>& matrix, const std::string& name) {
    std::cout << name << " dimensions: " << matrix.size() << "x" << matrix[0].size() << std::endl;
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    const int N = 500;
    const int M = 500;
    const int P = 500;
    
    std::cout << "Generating random matrices..." << std::endl;
    auto matrix_A = generate_random_matrix(N, M);
    auto matrix_B = generate_random_matrix(M, P);
    
    print_matrix_dimensions(matrix_A, "Matrix A");
    print_matrix_dimensions(matrix_B, "Matrix B");
    
    std::cout << "Performing parallel matrix multiplication..." << std::endl;
    double start_time = omp_get_wtime();
    
    auto result = multiply_matrices_parallel(matrix_A, matrix_B);
    
    double end_time = omp_get_wtime();
    double elapsed_time = end_time - start_time;
    
    print_matrix_dimensions(result, "Result matrix");
    std::cout << "Computation time: " << elapsed_time << " seconds" << std::endl;
    
    double sample_value = result[N/2][P/2];
    std::cout << "Sample value at center: " << sample_value << std::endl;
    
    return 0;
}