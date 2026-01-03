#include <iostream>
#include <stdexcept>

class Matrix {
private:
    int rows;
    int cols;
    double** data;

    void allocateMemory() {
        data = new double*[rows];
        for (int i = 0; i < rows; ++i) {
            data[i] = new double[cols]();
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
    }

    ~Matrix() {
        deallocateMemory();
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

    void setValue(int i, int j, double value) {
        if (i < 0 || i >= rows || j < 0 || j >= cols) {
            throw std::out_of_range("Matrix index out of range");
        }
        data[i][j] = value;
    }

    double getValue(int i, int j) const {
        if (i < 0 || i >= rows || j < 0 || j >= cols) {
            throw std::out_of_range("Matrix index out of range");
        }
        return data[i][j];
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

    void print() const {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                std::cout << data[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    try {
        Matrix mat(3, 4);
        
        double counter = 1.0;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 4; ++j) {
                mat.setValue(i, j, counter);
                counter += 1.0;
            }
        }

        std::cout << "Original matrix:" << std::endl;
        mat.print();

        Matrix transposed = mat.transpose();
        
        std::cout << "\nTransposed matrix:" << std::endl;
        transposed.print();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}