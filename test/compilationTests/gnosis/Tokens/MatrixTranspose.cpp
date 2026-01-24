
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

    void fillMatrix() {
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
        for (const auto& row : data) {
            for (int val : row) {
                std::cout << val << " ";
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    Matrix original(3, 4);
    original.fillMatrix();
    
    std::cout << "Original Matrix:" << std::endl;
    original.print();
    
    Matrix transposed = original.transpose();
    
    std::cout << "\nTransposed Matrix:" << std::endl;
    transposed.print();
    
    return 0;
}#include <iostream>
#include <iomanip>

const int ROWS = 3;
const int COLS = 4;

void transposeMatrix(int src[ROWS][COLS], int dst[COLS][ROWS]) {
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            dst[j][i] = src[i][j];
        }
    }
}

void printMatrix(int matrix[][COLS], int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            std::cout << std::setw(4) << matrix[i][j];
        }
        std::cout << std::endl;
    }
}

void printTransposedMatrix(int matrix[][ROWS], int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            std::cout << std::setw(4) << matrix[i][j];
        }
        std::cout << std::endl;
    }
}

int main() {
    int originalMatrix[ROWS][COLS] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12}
    };

    int transposedMatrix[COLS][ROWS];

    std::cout << "Original Matrix (" << ROWS << "x" << COLS << "):" << std::endl;
    printMatrix(originalMatrix, ROWS, COLS);

    transposeMatrix(originalMatrix, transposedMatrix);

    std::cout << "\nTransposed Matrix (" << COLS << "x" << ROWS << "):" << std::endl;
    printTransposedMatrix(transposedMatrix, COLS, ROWS);

    return 0;
}