
#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <omp.h>

std::vector<std::vector<double>> generateRandomMatrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
        }
    }
    return matrix;
}

std::vector<std::vector<double>> multiplyMatrices(
    const std::vector<std::vector<double>>& A,
    const std::vector<std::vector<double>>& B) {
    
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();
    
    std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));
    
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

void printMatrixStats(const std::vector<std::vector<double>>& matrix) {
    double sum = 0.0;
    double minVal = matrix[0][0];
    double maxVal = matrix[0][0];
    
    #pragma omp parallel for reduction(+:sum) reduction(min:minVal) reduction(max:maxVal) collapse(2)
    for (size_t i = 0; i < matrix.size(); ++i) {
        for (size_t j = 0; j < matrix[i].size(); ++j) {
            double val = matrix[i][j];
            sum += val;
            if (val < minVal) minVal = val;
            if (val > maxVal) maxVal = val;
        }
    }
    
    std::cout << "Matrix statistics:" << std::endl;
    std::cout << "  Dimensions: " << matrix.size() << "x" << matrix[0].size() << std::endl;
    std::cout << "  Sum of all elements: " << sum << std::endl;
    std::cout << "  Minimum value: " << minVal << std::endl;
    std::cout << "  Maximum value: " << maxVal << std::endl;
}

int main() {
    const int SIZE = 500;
    
    srand(42);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::vector<double>> matrixA = generateRandomMatrix(SIZE, SIZE);
    std::vector<std::vector<double>> matrixB = generateRandomMatrix(SIZE, SIZE);
    
    auto genEnd = std::chrono::high_resolution_clock::now();
    
    std::vector<std::vector<double>> result = multiplyMatrices(matrixA, matrixB);
    
    auto multEnd = std::chrono::high_resolution_clock::now();
    
    auto genDuration = std::chrono::duration_cast<std::chrono::milliseconds>(genEnd - start);
    auto multDuration = std::chrono::duration_cast<std::chrono::milliseconds>(multEnd - genEnd);
    auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(multEnd - start);
    
    std::cout << "Matrix multiplication completed!" << std::endl;
    std::cout << "Generation time: " << genDuration.count() << " ms" << std::endl;
    std::cout << "Multiplication time: " << multDuration.count() << " ms" << std::endl;
    std::cout << "Total execution time: " << totalDuration.count() << " ms" << std::endl;
    
    printMatrixStats(result);
    
    return 0;
}
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