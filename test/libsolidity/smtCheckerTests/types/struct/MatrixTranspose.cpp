#include <iostream>
#include <vector>
#include <type_traits>

template<typename T>
class Matrix {
private:
    std::vector<std::vector<T>> data;
    size_t rows;
    size_t cols;

public:
    Matrix(size_t r, size_t c) : rows(r), cols(c) {
        data.resize(rows, std::vector<T>(cols));
    }

    Matrix(const std::vector<std::vector<T>>& input) : data(input) {
        rows = data.size();
        cols = rows > 0 ? data[0].size() : 0;
    }

    void setElement(size_t i, size_t j, T value) {
        if (i < rows && j < cols) {
            data[i][j] = value;
        }
    }

    T getElement(size_t i, size_t j) const {
        if (i < rows && j < cols) {
            return data[i][j];
        }
        return T{};
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

    Matrix<T> transpose() const {
        Matrix<T> result(cols, rows);
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                result.setElement(j, i, data[i][j]);
            }
        }
        return result;
    }

    template<typename U = T>
    typename std::enable_if<std::is_arithmetic<U>::value, Matrix<U>>::type
    scalarMultiply(U scalar) const {
        Matrix<U> result(rows, cols);
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                result.setElement(i, j, data[i][j] * scalar);
            }
        }
        return result;
    }
};

template<typename T>
void demonstrateMatrixOperations() {
    std::vector<std::vector<T>> initial = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };

    Matrix<T> original(initial);
    std::cout << "Original matrix:" << std::endl;
    original.print();

    Matrix<T> transposed = original.transpose();
    std::cout << "\nTransposed matrix:" << std::endl;
    transposed.print();

    if (std::is_arithmetic<T>::value) {
        Matrix<T> scaled = original.scalarMultiply(static_cast<T>(2));
        std::cout << "\nScaled matrix (multiplied by 2):" << std::endl;
        scaled.print();
    }
}

int main() {
    std::cout << "=== Integer Matrix ===" << std::endl;
    demonstrateMatrixOperations<int>();

    std::cout << "\n=== Double Matrix ===" << std::endl;
    demonstrateMatrixOperations<double>();

    std::cout << "\n=== Float Matrix ===" << std::endl;
    demonstrateMatrixOperations<float>();

    return 0;
}