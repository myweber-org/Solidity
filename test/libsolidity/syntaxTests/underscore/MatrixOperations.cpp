
#include <iostream>
#include <vector>
#include <stdexcept>
#include <cmath>

class Matrix {
private:
    std::vector<std::vector<double>> data;
    size_t rows;
    size_t cols;

public:
    Matrix(size_t r, size_t c) : rows(r), cols(c) {
        data.resize(rows, std::vector<double>(cols, 0.0));
    }

    Matrix(const std::vector<std::vector<double>>& input) {
        if (input.empty() || input[0].empty()) {
            throw std::invalid_argument("Matrix cannot be empty");
        }
        rows = input.size();
        cols = input[0].size();
        data = input;
        for (const auto& row : data) {
            if (row.size() != cols) {
                throw std::invalid_argument("All rows must have the same number of columns");
            }
        }
    }

    double& operator()(size_t i, size_t j) {
        if (i >= rows || j >= cols) {
            throw std::out_of_range("Matrix indices out of range");
        }
        return data[i][j];
    }

    const double& operator()(size_t i, size_t j) const {
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

    Matrix multiply(const Matrix& other) const {
        if (cols != other.rows) {
            throw std::invalid_argument("Matrix dimensions do not match for multiplication");
        }
        Matrix result(rows, other.cols);
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < other.cols; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < cols; ++k) {
                    sum += data[i][k] * other.data[k][j];
                }
                result(i, j) = sum;
            }
        }
        return result;
    }

    double determinant() const {
        if (rows != cols) {
            throw std::invalid_argument("Determinant can only be calculated for square matrices");
        }
        if (rows == 1) {
            return data[0][0];
        }
        if (rows == 2) {
            return data[0][0] * data[1][1] - data[0][1] * data[1][0];
        }
        double det = 0.0;
        for (size_t j = 0; j < cols; ++j) {
            Matrix submatrix(rows - 1, cols - 1);
            for (size_t i = 1; i < rows; ++i) {
                size_t colIndex = 0;
                for (size_t j2 = 0; j2 < cols; ++j2) {
                    if (j2 == j) continue;
                    submatrix(i - 1, colIndex) = data[i][j2];
                    ++colIndex;
                }
            }
            double subDet = submatrix.determinant();
            det += (j % 2 == 0 ? 1 : -1) * data[0][j] * subDet;
        }
        return det;
    }
};

int main() {
    try {
        std::vector<std::vector<double>> matA = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
        std::vector<std::vector<double>> matB = {{9, 8, 7}, {6, 5, 4}, {3, 2, 1}};
        
        Matrix A(matA);
        Matrix B(matB);
        
        std::cout << "Matrix A:" << std::endl;
        A.print();
        std::cout << "Matrix B:" << std::endl;
        B.print();
        
        Matrix C = A.multiply(B);
        std::cout << "Matrix A * B:" << std::endl;
        C.print();
        
        double detA = A.determinant();
        std::cout << "Determinant of A: " << detA << std::endl;
        
        std::vector<std::vector<double>> mat2x2 = {{1, 2}, {3, 4}};
        Matrix D(mat2x2);
        std::cout << "2x2 Matrix:" << std::endl;
        D.print();
        std::cout << "Determinant: " << D.determinant() << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}