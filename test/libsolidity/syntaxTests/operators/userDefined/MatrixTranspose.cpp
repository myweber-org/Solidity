
#include <iostream>
#include <vector>

class Matrix {
private:
    std::vector<std::vector<int>> data;
    size_t rows;
    size_t cols;

public:
    Matrix(size_t r, size_t c) : rows(r), cols(c) {
        data.resize(rows, std::vector<int>(cols, 0));
    }

    void fillMatrix() {
        int counter = 1;
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                data[i][j] = counter++;
            }
        }
    }

    Matrix transpose() const {
        Matrix result(cols, rows);
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
    const size_t ROWS = 3;
    const size_t COLS = 4;

    Matrix original(ROWS, COLS);
    original.fillMatrix();

    std::cout << "Original matrix (" << ROWS << "x" << COLS << "):" << std::endl;
    original.print();

    Matrix transposed = original.transpose();
    std::cout << "\nTransposed matrix (" << COLS << "x" << ROWS << "):" << std::endl;
    transposed.print();

    return 0;
}