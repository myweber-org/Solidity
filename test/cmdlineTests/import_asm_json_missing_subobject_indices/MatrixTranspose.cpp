#include <iostream>
#include <cstdlib>

double** allocateMatrix(int rows, int cols) {
    double** matrix = new double*[rows];
    for (int i = 0; i < rows; ++i) {
        matrix[i] = new double[cols];
    }
    return matrix;
}

void deallocateMatrix(double** matrix, int rows) {
    for (int i = 0; i < rows; ++i) {
        delete[] matrix[i];
    }
    delete[] matrix;
}

double** transposeMatrix(double** matrix, int rows, int cols) {
    double** result = allocateMatrix(cols, rows);
    
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            result[j][i] = matrix[i][j];
        }
    }
    
    return result;
}

void printMatrix(double** matrix, int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            std::cout << matrix[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    const int rows = 3;
    const int cols = 4;
    
    double** original = allocateMatrix(rows, cols);
    
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            original[i][j] = i * cols + j + 1;
        }
    }
    
    std::cout << "Original matrix:" << std::endl;
    printMatrix(original, rows, cols);
    
    double** transposed = transposeMatrix(original, rows, cols);
    
    std::cout << "\nTransposed matrix:" << std::endl;
    printMatrix(transposed, cols, rows);
    
    deallocateMatrix(original, rows);
    deallocateMatrix(transposed, cols);
    
    return 0;
}