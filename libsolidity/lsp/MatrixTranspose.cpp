
#include <iostream>
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

    void setValue(size_t r, size_t c, T value) {
        if (r < rows && c < cols) {
            data[r][c] = value;
        }
    }

    T getValue(size_t r, size_t c) const {
        if (r < rows && c < cols) {
            return data[r][c];
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
                result.setValue(j, i, data[i][j]);
            }
        }
        return result;
    }
};

int main() {
    Matrix<int> mat(3, 4);
    
    int counter = 1;
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 4; ++j) {
            mat.setValue(i, j, counter++);
        }
    }

    std::cout << "Original Matrix:" << std::endl;
    mat.print();

    Matrix<int> transposed = mat.transpose();
    
    std::cout << "\nTransposed Matrix:" << std::endl;
    transposed.print();

    return 0;
}