#include <iostream>
#include <Eigen/Dense>

int main() {
    const int rows = 3;
    const int cols = 3;

    Eigen::MatrixXd matrixA = Eigen::MatrixXd::Random(rows, cols);
    Eigen::MatrixXd matrixB = Eigen::MatrixXd::Random(rows, cols);

    std::cout << "Matrix A:\n" << matrixA << "\n\n";
    std::cout << "Matrix B:\n" << matrixB << "\n\n";

    if(matrixA.cols() != matrixB.rows()) {
        std::cerr << "Error: Matrix dimensions mismatch for multiplication.\n";
        return 1;
    }

    Eigen::MatrixXd result = matrixA * matrixB;
    std::cout << "Result of A * B:\n" << result << std::endl;

    return 0;
}