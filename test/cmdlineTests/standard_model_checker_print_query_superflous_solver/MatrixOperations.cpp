
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

Matrix addMatrices(const Matrix& a, const Matrix& b) {
    if (a.getRows() != b.getRows() || a.getCols() != b.getCols()) {
        throw std::invalid_argument("Matrix dimensions must match for addition");
    }

    Matrix result(a.getRows(), a.getCols());
    for (size_t i = 0; i < a.getRows(); ++i) {
        for (size_t j = 0; j < a.getCols(); ++j) {
            result.setValue(i, j, a.getValue(i, j) + b.getValue(i, j));
        }
    }
    return result;
}

Matrix multiplyMatrices(const Matrix& a, const Matrix& b) {
    if (a.getCols() != b.getRows()) {
        throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
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
        Matrix mat1(2, 3);
        mat1.setValue(0, 0, 1.0);
        mat1.setValue(0, 1, 2.0);
        mat1.setValue(0, 2, 3.0);
        mat1.setValue(1, 0, 4.0);
        mat1.setValue(1, 1, 5.0);
        mat1.setValue(1, 2, 6.0);

        Matrix mat2(2, 3);
        mat2.setValue(0, 0, 7.0);
        mat2.setValue(0, 1, 8.0);
        mat2.setValue(0, 2, 9.0);
        mat2.setValue(1, 0, 10.0);
        mat2.setValue(1, 1, 11.0);
        mat2.setValue(1, 2, 12.0);

        std::cout << "Matrix 1:" << std::endl;
        mat1.print();
        std::cout << "Matrix 2:" << std::endl;
        mat2.print();

        Matrix sum = addMatrices(mat1, mat2);
        std::cout << "Sum of matrices:" << std::endl;
        sum.print();

        Matrix mat3(3, 2);
        mat3.setValue(0, 0, 1.0);
        mat3.setValue(0, 1, 2.0);
        mat3.setValue(1, 0, 3.0);
        mat3.setValue(1, 1, 4.0);
        mat3.setValue(2, 0, 5.0);
        mat3.setValue(2, 1, 6.0);

        std::cout << "Matrix 3:" << std::endl;
        mat3.print();

        Matrix product = multiplyMatrices(mat1, mat3);
        std::cout << "Product of matrix 1 and matrix 3:" << std::endl;
        product.print();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}