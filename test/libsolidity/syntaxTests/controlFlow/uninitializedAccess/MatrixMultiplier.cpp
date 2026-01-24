
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
    try {
        Matrix matA(2, 3);
        Matrix matB(3, 2);

        double val = 1.0;
        for (size_t i = 0; i < matA.getRows(); ++i) {
            for (size_t j = 0; j < matA.getCols(); ++j) {
                matA.setValue(i, j, val++);
            }
        }

        val = 7.0;
        for (size_t i = 0; i < matB.getRows(); ++i) {
            for (size_t j = 0; j < matB.getCols(); ++j) {
                matB.setValue(i, j, val--);
            }
        }

        std::cout << "Matrix A:" << std::endl;
        matA.print();
        std::cout << "Matrix B:" << std::endl;
        matB.print();

        Matrix matC = multiply(matA, matB);
        std::cout << "Result of multiplication (A * B):" << std::endl;
        matC.print();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}#include <iostream>
#include <vector>
#include <stdexcept>

template<typename T>
std::vector<std::vector<T>> multiplyMatrices(const std::vector<std::vector<T>>& matrixA,
                                             const std::vector<std::vector<T>>& matrixB) {
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
        throw std::invalid_argument("Number of columns in Matrix A must equal number of rows in Matrix B.");
    }

    std::vector<std::vector<T>> result(rowsA, std::vector<T>(colsB, T()));

    for (size_t i = 0; i < rowsA; ++i) {
        for (size_t j = 0; j < colsB; ++j) {
            T sum = T();
            for (size_t k = 0; k < colsA; ++k) {
                sum += matrixA[i][k] * matrixB[k][j];
            }
            result[i][j] = sum;
        }
    }

    return result;
}

template<typename T>
void printMatrix(const std::vector<std::vector<T>>& matrix) {
    for (const auto& row : matrix) {
        for (const auto& elem : row) {
            std::cout << elem << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    try {
        std::vector<std::vector<int>> A = {{1, 2, 3}, {4, 5, 6}};
        std::vector<std::vector<int>> B = {{7, 8}, {9, 10}, {11, 12}};

        std::cout << "Matrix A:" << std::endl;
        printMatrix(A);
        std::cout << "Matrix B:" << std::endl;
        printMatrix(B);

        auto C = multiplyMatrices(A, B);
        std::cout << "Result of A * B:" << std::endl;
        printMatrix(C);

        std::vector<std::vector<double>> D = {{1.5, 2.5}, {3.5, 4.5}};
        std::vector<std::vector<double>> E = {{0.5, 1.0}, {1.5, 2.0}};

        std::cout << "\nMatrix D:" << std::endl;
        printMatrix(D);
        std::cout << "Matrix E:" << std::endl;
        printMatrix(E);

        auto F = multiplyMatrices(D, E);
        std::cout << "Result of D * E:" << std::endl;
        printMatrix(F);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}