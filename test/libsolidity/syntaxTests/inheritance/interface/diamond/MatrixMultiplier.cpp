
#include <iostream>
#include <vector>

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
        if (i < rows && j < cols) {
            data[i][j] = value;
        }
    }

    double getValue(size_t i, size_t j) const {
        if (i < rows && j < cols) {
            return data[i][j];
        }
        return 0.0;
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
        throw std::invalid_argument("Matrix dimensions do not match for multiplication.");
    }

    size_t rows = a.getRows();
    size_t cols = b.getCols();
    size_t inner = a.getCols();

    Matrix result(rows, cols);

    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            double sum = 0.0;
            for (size_t k = 0; k < inner; ++k) {
                sum += a.getValue(i, k) * b.getValue(k, j);
            }
            result.setValue(i, j, sum);
        }
    }

    return result;
}

int main() {
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

    try {
        Matrix matC = multiply(matA, matB);
        std::cout << "Result of multiplication:" << std::endl;
        matC.print();
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}