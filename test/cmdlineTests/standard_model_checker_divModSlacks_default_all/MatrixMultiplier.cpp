#include <iostream>
#include <vector>
#include <stdexcept>

std::vector<std::vector<int>> multiplyMatrices(const std::vector<std::vector<int>>& matrixA, const std::vector<std::vector<int>>& matrixB) {
    if (matrixA.empty() || matrixB.empty()) {
        throw std::invalid_argument("Input matrices cannot be empty.");
    }
    
    size_t rowsA = matrixA.size();
    size_t colsA = matrixA[0].size();
    size_t rowsB = matrixB.size();
    size_t colsB = matrixB[0].size();
    
    for (const auto& row : matrixA) {
        if (row.size() != colsA) {
            throw std::invalid_argument("Matrix A rows have inconsistent sizes.");
        }
    }
    
    for (const auto& row : matrixB) {
        if (row.size() != colsB) {
            throw std::invalid_argument("Matrix B rows have inconsistent sizes.");
        }
    }
    
    if (colsA != rowsB) {
        throw std::invalid_argument("Matrix dimensions are incompatible for multiplication.");
    }
    
    std::vector<std::vector<int>> result(rowsA, std::vector<int>(colsB, 0));
    
    for (size_t i = 0; i < rowsA; ++i) {
        for (size_t j = 0; j < colsB; ++j) {
            for (size_t k = 0; k < colsA; ++k) {
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
    try {
        std::vector<std::vector<int>> A = {{1, 2, 3}, {4, 5, 6}};
        std::vector<std::vector<int>> B = {{7, 8}, {9, 10}, {11, 12}};
        
        std::vector<std::vector<int>> C = multiplyMatrices(A, B);
        
        std::cout << "Matrix A:" << std::endl;
        printMatrix(A);
        std::cout << "Matrix B:" << std::endl;
        printMatrix(B);
        std::cout << "Result of A * B:" << std::endl;
        printMatrix(C);
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}