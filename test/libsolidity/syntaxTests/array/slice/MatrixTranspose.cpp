
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

    void setValue(size_t r, size_t c, double value) {
        if (r < rows && c < cols) {
            data[r][c] = value;
        }
    }

    double getValue(size_t r, size_t c) const {
        if (r < rows && c < cols) {
            return data[r][c];
        }
        return 0.0;
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

    void print() const {
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                std::cout << data[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }

    size_t getRows() const { return rows; }
    size_t getCols() const { return cols; }
};

int main() {
    Matrix mat(3, 4);
    
    double counter = 1.0;
    for (size_t i = 0; i < mat.getRows(); ++i) {
        for (size_t j = 0; j < mat.getCols(); ++j) {
            mat.setValue(i, j, counter);
            counter += 1.0;
        }
    }

    std::cout << "Original Matrix:" << std::endl;
    mat.print();

    Matrix transposed = mat.transpose();
    std::cout << "\nTransposed Matrix:" << std::endl;
    transposed.print();

    return 0;
}