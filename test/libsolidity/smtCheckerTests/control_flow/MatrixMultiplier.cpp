
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

class Matrix {
private:
    std::vector<std::vector<double>> data;
    size_t rows;
    size_t cols;

public:
    Matrix(size_t r, size_t c) : rows(r), cols(c) {
        data.resize(rows);
        for (size_t i = 0; i < rows; ++i) {
            data[i].resize(cols, 0.0);
        }
    }

    void randomize() {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                data[i][j] = static_cast<double>(std::rand()) / RAND_MAX * 100.0;
            }
        }
    }

    void display() const {
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                std::cout << data[i][j] << "\t";
            }
            std::cout << std::endl;
        }
    }

    Matrix multiply(const Matrix& other) const {
        if (cols != other.rows) {
            throw std::invalid_argument("Matrix dimensions mismatch for multiplication");
        }

        Matrix result(rows, other.cols);
        for (size_t i = 0; i < rows; ++i) {
            for (size_t k = 0; k < cols; ++k) {
                for (size_t j = 0; j < other.cols; ++j) {
                    result.data[i][j] += data[i][k] * other.data[k][j];
                }
            }
        }
        return result;
    }

    size_t getRows() const { return rows; }
    size_t getCols() const { return cols; }
};

int main() {
    const size_t rowsA = 3;
    const size_t colsA = 4;
    const size_t rowsB = 4;
    const size_t colsB = 2;

    Matrix matA(rowsA, colsA);
    Matrix matB(rowsB, colsB);

    matA.randomize();
    matB.randomize();

    std::cout << "Matrix A (" << rowsA << "x" << colsA << "):" << std::endl;
    matA.display();
    std::cout << std::endl;

    std::cout << "Matrix B (" << rowsB << "x" << colsB << "):" << std::endl;
    matB.display();
    std::cout << std::endl;

    try {
        Matrix matC = matA.multiply(matB);
        std::cout << "Result Matrix C (" << matC.getRows() << "x" << matC.getCols() << "):" << std::endl;
        matC.display();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}