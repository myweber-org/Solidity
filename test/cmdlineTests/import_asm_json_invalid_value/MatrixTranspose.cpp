#include <iostream>
#include <vector>

std::vector<std::vector<int>> transposeMatrix(const std::vector<std::vector<int>>& matrix) {
    if (matrix.empty()) return {};

    size_t rows = matrix.size();
    size_t cols = matrix[0].size();

    std::vector<std::vector<int>> result(cols, std::vector<int>(rows));

    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            result[j][i] = matrix[i][j];
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
    std::vector<std::vector<int>> original = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };

    std::cout << "Original matrix:" << std::endl;
    printMatrix(original);

    std::vector<std::vector<int>> transposed = transposeMatrix(original);

    std::cout << "\nTransposed matrix:" << std::endl;
    printMatrix(transposed);

    return 0;
}#include <iostream>
#include <vector>

template<typename T>
class Matrix {
private:
    std::vector<std::vector<T>> data;
    size_t rows;
    size_t cols;

public:
    Matrix(size_t r, size_t c) : rows(r), cols(c) {
        data.resize(rows, std::vector<T>(cols, T{}));
    }

    void fillRandom() {
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                data[i][j] = static_cast<T>(rand() % 100);
            }
        }
    }

    Matrix<T> transpose() const {
        Matrix<T> result(cols, rows);
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                result.data[j][i] = data[i][j];
            }
        }
        return result;
    }

    void print() const {
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                std::cout << data[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }

    size_t getRows() const { return rows; }
    size_t getCols() const { return cols; }
};

int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    Matrix<int> original(3, 4);
    original.fillRandom();

    std::cout << "Original Matrix (" << original.getRows() << "x" << original.getCols() << "):" << std::endl;
    original.print();

    Matrix<int> transposed = original.transpose();
    std::cout << "\nTransposed Matrix (" << transposed.getRows() << "x" << transposed.getCols() << "):" << std::endl;
    transposed.print();

    return 0;
}