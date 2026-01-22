#include <iostream>
#include <iomanip>

class Matrix3x3 {
private:
    double data[3][3];

public:
    Matrix3x3() {
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                data[i][j] = 0.0;
            }
        }
    }

    Matrix3x3(double values[3][3]) {
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                data[i][j] = values[i][j];
            }
        }
    }

    void setValue(int row, int col, double value) {
        if (row >= 0 && row < 3 && col >= 0 && col < 3) {
            data[row][col] = value;
        }
    }

    double getValue(int row, int col) const {
        if (row >= 0 && row < 3 && col >= 0 && col < 3) {
            return data[row][col];
        }
        return 0.0;
    }

    void print() const {
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                std::cout << std::setw(10) << data[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }

    Matrix3x3 multiply(const Matrix3x3& other) const {
        Matrix3x3 result;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                double sum = 0.0;
                for (int k = 0; k < 3; ++k) {
                    sum += data[i][k] * other.data[k][j];
                }
                result.data[i][j] = sum;
            }
        }
        return result;
    }
};

int main() {
    double matAValues[3][3] = {
        {1.0, 2.0, 3.0},
        {4.0, 5.0, 6.0},
        {7.0, 8.0, 9.0}
    };

    double matBValues[3][3] = {
        {9.0, 8.0, 7.0},
        {6.0, 5.0, 4.0},
        {3.0, 2.0, 1.0}
    };

    Matrix3x3 matA(matAValues);
    Matrix3x3 matB(matBValues);

    std::cout << "Matrix A:" << std::endl;
    matA.print();
    std::cout << std::endl;

    std::cout << "Matrix B:" << std::endl;
    matB.print();
    std::cout << std::endl;

    Matrix3x3 matC = matA.multiply(matB);

    std::cout << "Result of A * B:" << std::endl;
    matC.print();

    return 0;
}