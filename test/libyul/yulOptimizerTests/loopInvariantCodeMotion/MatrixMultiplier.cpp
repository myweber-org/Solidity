#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

void initializeMatrix(std::vector<std::vector<double>>& matrix, int rows, int cols) {
    matrix.resize(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
        }
    }
}

void multiplyMatrices(const std::vector<std::vector<double>>& A,
                      const std::vector<std::vector<double>>& B,
                      std::vector<std::vector<double>>& C,
                      int rowsA, int colsA, int colsB) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            double sum = 0.0;
            for (int k = 0; k < colsA; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

void printMatrix(const std::vector<std::vector<double>>& matrix, int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            std::cout << matrix[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    const int ROWS_A = 500;
    const int COLS_A = 500;
    const int COLS_B = 500;

    srand(static_cast<unsigned>(time(nullptr)));

    std::vector<std::vector<double>> A, B, C;

    std::cout << "Initializing matrices..." << std::endl;
    initializeMatrix(A, ROWS_A, COLS_A);
    initializeMatrix(B, COLS_A, COLS_B);
    C.resize(ROWS_A, std::vector<double>(COLS_B));

    std::cout << "Performing matrix multiplication..." << std::endl;
    double start_time = omp_get_wtime();
    multiplyMatrices(A, B, C, ROWS_A, COLS_A, COLS_B);
    double end_time = omp_get_wtime();

    std::cout << "Multiplication completed in " << (end_time - start_time) << " seconds." << std::endl;

    return 0;
}