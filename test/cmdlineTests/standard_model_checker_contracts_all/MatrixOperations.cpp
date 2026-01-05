#include <iostream>
#include <vector>

class Matrix {
private:
    std::vector<std::vector<int>> data;
    int rows;
    int cols;

public:
    Matrix(int r, int c) : rows(r), cols(c) {
        data.resize(rows, std::vector<int>(cols, 0));
    }

    void fillFromInput() {
        std::cout << "Enter matrix elements (" << rows << "x" << cols << "):\n";
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                std::cin >> data[i][j];
            }
        }
    }

    void fillRandom() {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                data[i][j] = rand() % 10;
            }
        }
    }

    void display() const {
        for (const auto& row : data) {
            for (int val : row) {
                std::cout << val << " ";
            }
            std::cout << "\n";
        }
    }

    Matrix add(const Matrix& other) const {
        if (rows != other.rows || cols != other.cols) {
            throw std::invalid_argument("Matrix dimensions must match for addition.");
        }
        Matrix result(rows, cols);
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                result.data[i][j] = data[i][j] + other.data[i][j];
            }
        }
        return result;
    }

    Matrix subtract(const Matrix& other) const {
        if (rows != other.rows || cols != other.cols) {
            throw std::invalid_argument("Matrix dimensions must match for subtraction.");
        }
        Matrix result(rows, cols);
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                result.data[i][j] = data[i][j] - other.data[i][j];
            }
        }
        return result;
    }

    Matrix multiply(const Matrix& other) const {
        if (cols != other.rows) {
            throw std::invalid_argument("Number of columns in first matrix must equal number of rows in second matrix.");
        }
        Matrix result(rows, other.cols);
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < other.cols; ++j) {
                for (int k = 0; k < cols; ++k) {
                    result.data[i][j] += data[i][k] * other.data[k][j];
                }
            }
        }
        return result;
    }
};

int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    Matrix matA(2, 3);
    Matrix matB(2, 3);
    Matrix matC(3, 2);

    std::cout << "Matrix A (random):\n";
    matA.fillRandom();
    matA.display();

    std::cout << "\nMatrix B (random):\n";
    matB.fillRandom();
    matB.display();

    std::cout << "\nMatrix C (random):\n";
    matC.fillRandom();
    matC.display();

    try {
        std::cout << "\nA + B:\n";
        Matrix sum = matA.add(matB);
        sum.display();

        std::cout << "\nA - B:\n";
        Matrix diff = matA.subtract(matB);
        diff.display();

        std::cout << "\nA * C:\n";
        Matrix prod = matA.multiply(matC);
        prod.display();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}