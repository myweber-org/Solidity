
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

    static Matrix add(const Matrix& a, const Matrix& b) {
        if (a.rows != b.rows || a.cols != b.cols) {
            throw std::invalid_argument("Matrix dimensions must match for addition");
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
            throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
        }

        Matrix result(a.rows, b.cols);
        for (size_t i = 0; i < a.rows; ++i) {
            for (size_t j = 0; j < b.cols; ++j) {
                double sum = 0.0;
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
        Matrix matA(2, 3);
        matA.setValue(0, 0, 1.0);
        matA.setValue(0, 1, 2.0);
        matA.setValue(0, 2, 3.0);
        matA.setValue(1, 0, 4.0);
        matA.setValue(1, 1, 5.0);
        matA.setValue(1, 2, 6.0);

        Matrix matB(2, 3);
        matB.setValue(0, 0, 6.0);
        matB.setValue(0, 1, 5.0);
        matB.setValue(0, 2, 4.0);
        matB.setValue(1, 0, 3.0);
        matB.setValue(1, 1, 2.0);
        matB.setValue(1, 2, 1.0);

        std::cout << "Matrix A:" << std::endl;
        matA.print();
        std::cout << "Matrix B:" << std::endl;
        matB.print();

        Matrix sum = Matrix::add(matA, matB);
        std::cout << "Sum of A and B:" << std::endl;
        sum.print();

        Matrix matC(3, 2);
        matC.setValue(0, 0, 1.0);
        matC.setValue(0, 1, 2.0);
        matC.setValue(1, 0, 3.0);
        matC.setValue(1, 1, 4.0);
        matC.setValue(2, 0, 5.0);
        matC.setValue(2, 1, 6.0);

        std::cout << "Matrix C:" << std::endl;
        matC.print();

        Matrix product = Matrix::multiply(matA, matC);
        std::cout << "Product of A and C:" << std::endl;
        product.print();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}