
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
        if (r == 0 || c == 0) {
            throw std::invalid_argument("Matrix dimensions must be positive");
        }
        data.resize(r, std::vector<double>(c, 0.0));
    }

    size_t getRows() const { return rows; }
    size_t getCols() const { return cols; }

    double& operator()(size_t i, size_t j) {
        if (i >= rows || j >= cols) {
            throw std::out_of_range("Matrix index out of range");
        }
        return data[i][j];
    }

    const double& operator()(size_t i, size_t j) const {
        if (i >= rows || j >= cols) {
            throw std::out_of_range("Matrix index out of range");
        }
        return data[i][j];
    }

    void fillRandom() {
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                data[i][j] = static_cast<double>(rand() % 100) / 10.0;
            }
        }
    }

    void print() const {
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                std::cout << data[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }
};

Matrix multiplyMatrices(const Matrix& a, const Matrix& b) {
    if (a.getCols() != b.getRows()) {
        throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
    }

    size_t m = a.getRows();
    size_t n = a.getCols();
    size_t p = b.getCols();

    Matrix result(m, p);

    for (size_t i = 0; i < m; ++i) {
        for (size_t j = 0; j < p; ++j) {
            double sum = 0.0;
            for (size_t k = 0; k < n; ++k) {
                sum += a(i, k) * b(k, j);
            }
            result(i, j) = sum;
        }
    }

    return result;
}

int main() {
    try {
        srand(static_cast<unsigned>(time(nullptr)));

        Matrix matA(3, 2);
        Matrix matB(2, 4);

        matA.fillRandom();
        matB.fillRandom();

        std::cout << "Matrix A:" << std::endl;
        matA.print();
        std::cout << std::endl;

        std::cout << "Matrix B:" << std::endl;
        matB.print();
        std::cout << std::endl;

        Matrix matC = multiplyMatrices(matA, matB);

        std::cout << "Result of A * B:" << std::endl;
        matC.print();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}