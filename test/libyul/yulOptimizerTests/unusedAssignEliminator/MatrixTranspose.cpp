
#include <iostream>
#include <stdexcept>

class Matrix {
private:
    int rows;
    int cols;
    int** data;

public:
    Matrix(int r, int c) : rows(r), cols(c) {
        if (rows <= 0 || cols <= 0) {
            throw std::invalid_argument("Matrix dimensions must be positive");
        }
        
        data = new int*[rows];
        for (int i = 0; i < rows; ++i) {
            data[i] = new int[cols];
        }
    }

    ~Matrix() {
        for (int i = 0; i < rows; ++i) {
            delete[] data[i];
        }
        delete[] data;
    }

    void fillSequential() {
        int counter = 1;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                data[i][j] = counter++;
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

    void print() const {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                std::cout << data[i][j] << "\t";
            }
            std::cout << std::endl;
        }
    }

    int getRows() const { return rows; }
    int getCols() const { return cols; }
};

int main() {
    try {
        Matrix original(3, 4);
        original.fillSequential();
        
        std::cout << "Original matrix (" << original.getRows() 
                  << "x" << original.getCols() << "):" << std::endl;
        original.print();
        
        Matrix transposed = original.transpose();
        
        std::cout << "\nTransposed matrix (" << transposed.getRows() 
                  << "x" << transposed.getCols() << "):" << std::endl;
        transposed.print();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}