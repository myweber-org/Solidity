
#include <iostream>
#include <vector>

class Matrix {
private:
    int rows;
    int cols;
    std::vector<std::vector<int>> data;

public:
    Matrix(int r, int c) : rows(r), cols(c), data(r, std::vector<int>(c, 0)) {}

    void fillMatrix() {
        std::cout << "Enter matrix elements (" << rows << "x" << cols << "):\n";
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                std::cin >> data[i][j];
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
                std::cout << data[i][j] << " ";
            }
            std::cout << "\n";
        }
    }
};

int main() {
    int r, c;
    std::cout << "Enter number of rows: ";
    std::cin >> r;
    std::cout << "Enter number of columns: ";
    std::cin >> c;

    Matrix mat(r, c);
    mat.fillMatrix();

    std::cout << "\nOriginal Matrix:\n";
    mat.display();

    Matrix transposed = mat.transpose();
    std::cout << "\nTransposed Matrix:\n";
    transposed.display();

    return 0;
}