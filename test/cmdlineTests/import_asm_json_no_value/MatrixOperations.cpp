#include <iostream>
#include <vector>
#include <stdexcept>

class Matrix {
private:
    std::vector<std::vector<int>> data;
    size_t rows;
    size_t cols;

public:
    Matrix(size_t r, size_t c) : rows(r), cols(c) {
        data.resize(rows, std::vector<int>(cols, 0));
    }

    void setValue(size_t r, size_t c, int value) {
        if (r >= rows || c >= cols) {
            throw std::out_of_range("Matrix indices out of range");
        }
        data[r][c] = value;
    }

    int getValue(size_t r, size_t c) const {
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

    static Matrix add(const Matrix& a, const Matrix& b) {
        if (a.rows != b.rows || a.cols != b.cols) {
            throw std::invalid_argument("Matrices must have the same dimensions for addition");
        }

        Matrix result(a.rows, a.cols);
        for (size_t i = 0; i < a.rows; ++i) {
            for (size_t j = 0; j < a.cols; ++j) {
                result.data[i][j] = a.data[i][j] + b.data[i][j];
            }
        }
        return result;
    }

    static Matrix multiply(const Matrix& a, const Matrix& b) {
        if (a.cols != b.rows) {
            throw std::invalid_argument("Number of columns in first matrix must equal number of rows in second matrix");
        }

        Matrix result(a.rows, b.cols);
        for (size_t i = 0; i < a.rows; ++i) {
            for (size_t j = 0; j < b.cols; ++j) {
                int sum = 0;
                for (size_t k = 0; k < a.cols; ++k) {
                    sum += a.data[i][k] * b.data[k][j];
                }
                result.data[i][j] = sum;
            }
        }
        return result;
    }
};

int main() {
    try {
        Matrix mat1(2, 3);
        Matrix mat2(2, 3);
        Matrix mat3(3, 2);

        for (size_t i = 0; i < 2; ++i) {
            for (size_t j = 0; j < 3; ++j) {
                mat1.setValue(i, j, i + j);
                mat2.setValue(i, j, (i + j) * 2);
            }
        }

        for (size_t i = 0; i < 3; ++i) {
            for (size_t j = 0; j < 2; ++j) {
                mat3.setValue(i, j, i - j);
            }
        }

        std::cout << "Matrix 1:" << std::endl;
        mat1.print();
        std::cout << "Matrix 2:" << std::endl;
        mat2.print();

        Matrix sum = Matrix::add(mat1, mat2);
        std::cout << "Sum of Matrix 1 and Matrix 2:" << std::endl;
        sum.print();

        std::cout << "Matrix 3:" << std::endl;
        mat3.print();

        Matrix product = Matrix::multiply(mat1, mat3);
        std::cout << "Product of Matrix 1 and Matrix 3:" << std::endl;
        product.print();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}