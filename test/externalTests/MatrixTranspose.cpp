
#include <iostream>
#include <vector>
#include <stdexcept>

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

    void setElement(size_t r, size_t c, const T& value) {
        if (r >= rows || c >= cols) {
            throw std::out_of_range("Matrix indices out of range");
        }
        data[r][c] = value;
    }

    T getElement(size_t r, size_t c) const {
        if (r >= rows || c >= cols) {
            throw std::out_of_range("Matrix indices out of range");
        }
        return data[r][c];
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
    try {
        Matrix<int> mat(3, 4);
        
        int counter = 1;
        for (size_t i = 0; i < mat.getRows(); ++i) {
            for (size_t j = 0; j < mat.getCols(); ++j) {
                mat.setElement(i, j, counter++);
            }
        }

        std::cout << "Original Matrix:" << std::endl;
        mat.print();

        Matrix<int> transposed = mat.transpose();
        
        std::cout << "\nTransposed Matrix:" << std::endl;
        transposed.print();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}