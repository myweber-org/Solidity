#include <iostream>
#include <vector>
#include <stdexcept>

std::vector<std::vector<int>> transposeMatrix(const std::vector<std::vector<int>>& matrix) {
    if (matrix.empty()) {
        return {};
    }

    size_t rows = matrix.size();
    size_t cols = matrix[0].size();

    for (size_t i = 1; i < rows; ++i) {
        if (matrix[i].size() != cols) {
            throw std::invalid_argument("Input matrix is not rectangular");
        }
    }

    std::vector<std::vector<int>> result(cols, std::vector<int>(rows));

    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            result[j][i] = matrix[i][j];
        }
    }

    return result;
}

void printMatrix(const std::vector<std::vector<int>>& matrix) {
    for (const auto& row : matrix) {
        for (int val : row) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    std::vector<std::vector<int>> original = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };

    try {
        std::vector<std::vector<int>> transposed = transposeMatrix(original);

        std::cout << "Original matrix:" << std::endl;
        printMatrix(original);

        std::cout << "\nTransposed matrix:" << std::endl;
        printMatrix(transposed);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
#include <iostream>
#include <stdexcept>

class Matrix {
private:
    int** data;
    int rows;
    int cols;

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
        initializeWithDefault();
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

    void initializeWithDefault() {
        int counter = 1;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                data[i][j] = counter++;
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

void demonstrateMatrixTranspose() {
    const int originalRows = 3;
    const int originalCols = 4;
    
    std::cout << "Creating " << originalRows << "x" << originalCols << " matrix:" << std::endl;
    Matrix original(originalRows, originalCols);
    original.display();
    
    std::cout << "\nTransposed matrix:" << std::endl;
    Matrix transposed = original.transpose();
    transposed.display();
    
    std::cout << "\nVerifying transpose property:" << std::endl;
    bool correct = true;
    for (int i = 0; i < originalRows && correct; ++i) {
        for (int j = 0; j < originalCols && correct; ++j) {
            if (original.getValue(i, j) != transposed.getValue(j, i)) {
                correct = false;
            }
        }
    }
    
    if (correct) {
        std::cout << "Transpose operation verified successfully." << std::endl;
    } else {
        std::cout << "Transpose operation failed verification." << std::endl;
    }
}

int main() {
    try {
        demonstrateMatrixTranspose();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}