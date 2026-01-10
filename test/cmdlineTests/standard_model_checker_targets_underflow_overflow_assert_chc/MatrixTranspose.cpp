#include <iostream>
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
        for (int i = 0; i < rows; ++i) {
            delete[] data[i];
        }
        delete[] data;
    }

public:
    Matrix(int r, int c) : rows(r), cols(c) {
        if (rows <= 0 || cols <= 0) {
            throw std::invalid_argument("Matrix dimensions must be positive");
        }
        allocateMemory();
        initializeWithZeros();
    }

    ~Matrix() {
        deallocateMemory();
    }

    Matrix(const Matrix& other) : rows(other.rows), cols(other.cols) {
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

    void fillWithIncrement(int start = 1) {
        int value = start;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                data[i][j] = value++;
            }
        }
    }

    Matrix transpose() const {
        Matrix result(cols, rows);
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                result.data[j][i] = data[i][j];
            }
        }
        return result;
    }

    void display() const {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                std::cout << data[i][j] << "\t";
            }
            std::cout << std::endl;
        }
    }

    int getRowCount() const { return rows; }
    int getColCount() const { return cols; }
};

int main() {
    try {
        Matrix original(3, 4);
        original.fillWithIncrement();
        
        std::cout << "Original Matrix (" << original.getRowCount() 
                  << "x" << original.getColCount() << "):" << std::endl;
        original.display();
        
        Matrix transposed = original.transpose();
        
        std::cout << "\nTransposed Matrix (" << transposed.getRowCount() 
                  << "x" << transposed.getColCount() << "):" << std::endl;
        transposed.display();
        
        std::cout << "\nVerifying transpose property:" << std::endl;
        for (int i = 0; i < original.getRowCount(); ++i) {
            for (int j = 0; j < original.getColCount(); ++j) {
                if (original.getValue(i, j) != transposed.getValue(j, i)) {
                    std::cerr << "Transpose verification failed!" << std::endl;
                    return 1;
                }
            }
        }
        std::cout << "Transpose verified successfully." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}