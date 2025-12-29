
#include <iostream>
#include <vector>

class Matrix {
private:
    std::vector<std::vector<double>> data;
    size_t rows;
    size_t cols;

public:
    Matrix(size_t r, size_t c) : rows(r), cols(c) {
        data.resize(rows, std::vector<double>(cols, 0.0));
    }

    void setValue(size_t i, size_t j, double value) {
        if (i < rows && j < cols) {
            data[i][j] = value;
        }
    }

    double getValue(size_t i, size_t j) const {
        if (i < rows && j < cols) {
            return data[i][j];
        }
        return 0.0;
    }

    size_t getRows() const { return rows; }
    size_t getCols() const { return cols; }

    void display() const {
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                std::cout << data[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }

    Matrix transpose() const {
        Matrix result(cols, rows);
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                result.setValue(j, i, data[i][j]);
            }
        }
        return result;
    }
};

int main() {
    Matrix mat(3, 4);
    
    double counter = 1.0;
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 4; ++j) {
            mat.setValue(i, j, counter);
            counter += 1.0;
        }
    }

    std::cout << "Original Matrix:" << std::endl;
    mat.display();

    Matrix transposed = mat.transpose();
    std::cout << "\nTransposed Matrix:" << std::endl;
    transposed.display();

    return 0;
}