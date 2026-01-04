#include <iostream>
#include <array>

template<typename T, size_t N>
using Matrix = std::array<std::array<T, N>, N>;

template<typename T, size_t N>
Matrix<T, N> multiplyMatrices(const Matrix<T, N>& a, const Matrix<T, N>& b) {
    Matrix<T, N> result{};
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            T sum = 0;
            for (size_t k = 0; k < N; ++k) {
                sum += a[i][k] * b[k][j];
            }
            result[i][j] = sum;
        }
    }
    return result;
}

template<typename T, size_t N>
void printMatrix(const Matrix<T, N>& mat) {
    for (const auto& row : mat) {
        for (const auto& elem : row) {
            std::cout << elem << " ";
        }
        std::cout << "\n";
    }
}

int main() {
    constexpr size_t SIZE = 3;
    Matrix<int, SIZE> matA = {{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}};
    Matrix<int, SIZE> matB = {{{9, 8, 7}, {6, 5, 4}, {3, 2, 1}}};

    std::cout << "Matrix A:\n";
    printMatrix(matA);
    std::cout << "\nMatrix B:\n";
    printMatrix(matB);

    Matrix<int, SIZE> product = multiplyMatrices(matA, matB);
    std::cout << "\nProduct of A and B:\n";
    printMatrix(product);

    return 0;
}