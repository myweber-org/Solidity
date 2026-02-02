
#include <iostream>
#include <array>

class Matrix3x3 {
public:
    using Data = std::array<std::array<double, 3>, 3>;

    Matrix3x3() {
        for (auto& row : data) {
            row.fill(0.0);
        }
    }

    Matrix3x3(const Data& d) : data(d) {}

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
        for (const auto& row : data) {
            for (double val : row) {
                std::cout << val << " ";
            }
            std::cout << "\n";
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

private:
    Data data;
};

int main() {
    Matrix3x3::Data aData = {{
        {{1.0, 2.0, 3.0}},
        {{4.0, 5.0, 6.0}},
        {{7.0, 8.0, 9.0}}
    }};

    Matrix3x3::Data bData = {{
        {{9.0, 8.0, 7.0}},
        {{6.0, 5.0, 4.0}},
        {{3.0, 2.0, 1.0}}
    }};

    Matrix3x3 matA(aData);
    Matrix3x3 matB(bData);

    std::cout << "Matrix A:\n";
    matA.print();
    std::cout << "\nMatrix B:\n";
    matB.print();

    Matrix3x3 matC = matA.multiply(matB);
    std::cout << "\nResult of A * B:\n";
    matC.print();

    return 0;
}