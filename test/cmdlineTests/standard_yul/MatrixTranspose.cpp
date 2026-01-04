
#include <iostream>
#include <vector>

template <typename T>
class Matrix {
private:
    std::vector<std::vector<T>> data;
    size_t rows;
    size_t cols;

public:
    Matrix(size_t r, size_t c) : rows(r), cols(c) {
        data.resize(rows, std::vector<T>(cols, T()));
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

    std::cout << "Original matrix (" << original.getRows() 
              << "x" << original.getCols() << "):" << std::endl;
    original.print();

    Matrix<int> transposed = original.transpose();
    
    std::cout << "\nTransposed matrix (" << transposed.getRows() 
              << "x" << transposed.getCols() << "):" << std::endl;
    transposed.print();

    return 0;
}