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

    void fillFromInput() {
        std::cout << "Enter " << rows << "x" << cols << " matrix elements:\n";
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
            std::cout << "\n";
        }
    }

    size_t getRows() const { return rows; }
    size_t getCols() const { return cols; }
};

int main() {
    size_t r, c;
    std::cout << "Enter matrix dimensions (rows columns): ";
    std::cin >> r >> c;

    Matrix<int> mat(r, c);
    mat.fillFromInput();

    std::cout << "\nOriginal Matrix:\n";
    mat.display();

    Matrix<int> transposed = mat.transpose();
    std::cout << "\nTransposed Matrix:\n";
    transposed.display();

    return 0;
}#include <iostream>
#include <stdexcept>

class Matrix {
private:
    int rows;
    int cols;
    int** data;

    void allocateMemory() {
        data = new int*[rows];
        for (int i = 0; i < rows; ++i) {
            data[i] = new int[cols];
        }
    }

    void deallocateMemory() {
        if (data) {
            for (int i = 0; i < rows; ++i) {
                delete[] data[i];
            }
            delete[] data;
        }
    }

public:
    Matrix(int r, int c) : rows(r), cols(c), data(nullptr) {
        if (rows <= 0 || cols <= 0) {
            throw std::invalid_argument("Matrix dimensions must be positive");
        }
        allocateMemory();
        initializeWithZeros();
    }

    Matrix(const Matrix& other) : rows(other.rows), cols(other.cols), data(nullptr) {
        allocateMemory();
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                data[i][j] = other.data[i][j];
            }
        }
    }

    Matrix& operator=(const Matrix& other) {
        if (this != &other) {
            deallocateMemory();
            rows = other.rows;
            cols = other.cols;
            allocateMemory();
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    data[i][j] = other.data[i][j];
                }
            }
        }
        return *this;
    }

    ~Matrix() {
        deallocateMemory();
    }

    void initializeWithZeros() {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                data[i][j] = 0;
            }
        }
    }

    void setValue(int row, int col, int value) {
        if (row < 0 || row >= rows || col < 0 || col >= cols) {
            throw std::out_of_range("Matrix index out of range");
        }
        data[row][col] = value;
    }

    int getValue(int row, int col) const {
        if (row < 0 || row >= rows || col < 0 || col >= cols) {
            throw std::out_of_range("Matrix index out of range");
        }
        return data[row][col];
    }

    int getRows() const { return rows; }
    int getCols() const { return cols; }

    void print() const {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                std::cout << data[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }

    Matrix transpose() const {
        Matrix result(cols, rows);
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                result.setValue(j, i, data[i][j]);
            }
        }
        return result;
    }
};

int main() {
    try {
        Matrix mat(3, 4);
        
        int counter = 1;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 4; ++j) {
                mat.setValue(i, j, counter++);
            }
        }

        std::cout << "Original Matrix:" << std::endl;
        mat.print();

        Matrix transposed = mat.transpose();
        std::cout << "\nTransposed Matrix:" << std::endl;
        transposed.print();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}