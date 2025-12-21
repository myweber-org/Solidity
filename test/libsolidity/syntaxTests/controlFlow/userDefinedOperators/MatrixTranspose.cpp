
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
        data.resize(rows, std::vector<T>(cols, T()));
    }

    void fillFromInput() {
        std::cout << "Enter " << rows << "x" << cols << " matrix elements:" << std::endl;
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                std::cin >> data[i][j];
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

    void display() const {
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
    size_t rows, cols;
    
    std::cout << "Enter matrix dimensions (rows columns): ";
    std::cin >> rows >> cols;
    
    Matrix<int> matrix(rows, cols);
    matrix.fillFromInput();
    
    std::cout << "\nOriginal Matrix:" << std::endl;
    matrix.display();
    
    Matrix<int> transposed = matrix.transpose();
    
    std::cout << "\nTransposed Matrix:" << std::endl;
    transposed.display();
    
    return 0;
}