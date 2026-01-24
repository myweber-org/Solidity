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

    void setValue(int row, int col, int value) {
        if (row >= 0 && row < rows && col >= 0 && col < cols) {
            data[row][col] = value;
        }
    }

    int getValue(int row, int col) const {
        if (row >= 0 && row < rows && col >= 0 && col < cols) {
            return data[row][col];
        }
        return 0;
    }

    int getRows() const { return rows; }
    int getCols() const { return cols; }

    void display() const {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                std::cout << data[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }

    static Matrix add(const Matrix& m1, const Matrix& m2) {
        if (m1.rows != m2.rows || m1.cols != m2.cols) {
            throw std::invalid_argument("Matrices dimensions do not match for addition.");
        }

        Matrix result(m1.rows, m1.cols);
        for (int i = 0; i < m1.rows; ++i) {
            for (int j = 0; j < m1.cols; ++j) {
                result.data[i][j] = m1.data[i][j] + m2.data[i][j];
            }
        }
        return result;
    }

    static Matrix subtract(const Matrix& m1, const Matrix& m2) {
        if (m1.rows != m2.rows || m1.cols != m2.cols) {
            throw std::invalid_argument("Matrices dimensions do not match for subtraction.");
        }

        Matrix result(m1.rows, m1.cols);
        for (int i = 0; i < m1.rows; ++i) {
            for (int j = 0; j < m1.cols; ++j) {
                result.data[i][j] = m1.data[i][j] - m2.data[i][j];
            }
        }
        return result;
    }

    static Matrix multiply(const Matrix& m1, const Matrix& m2) {
        if (m1.cols != m2.rows) {
            throw std::invalid_argument("Matrices dimensions do not match for multiplication.");
        }

        Matrix result(m1.rows, m2.cols);
        for (int i = 0; i < m1.rows; ++i) {
            for (int j = 0; j < m2.cols; ++j) {
                for (int k = 0; k < m1.cols; ++k) {
                    result.data[i][j] += m1.data[i][k] * m2.data[k][j];
                }
            }
        }
        return result;
    }
};

int main() {
    Matrix mat1(2, 3);
    Matrix mat2(2, 3);
    Matrix mat3(3, 2);

    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 3; ++j) {
            mat1.setValue(i, j, i + j + 1);
            mat2.setValue(i, j, (i + j + 1) * 2);
        }
    }

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 2; ++j) {
            mat3.setValue(i, j, i + j + 1);
        }
    }

    std::cout << "Matrix 1:" << std::endl;
    mat1.display();
    std::cout << "Matrix 2:" << std::endl;
    mat2.display();
    std::cout << "Matrix 3:" << std::endl;
    mat3.display();

    try {
        Matrix sum = Matrix::add(mat1, mat2);
        std::cout << "Sum of Matrix 1 and Matrix 2:" << std::endl;
        sum.display();

        Matrix diff = Matrix::subtract(mat1, mat2);
        std::cout << "Difference of Matrix 1 and Matrix 2:" << std::endl;
        diff.display();

        Matrix product = Matrix::multiply(mat1, mat3);
        std::cout << "Product of Matrix 1 and Matrix 3:" << std::endl;
        product.display();
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}