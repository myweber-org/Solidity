
#include <iostream>
#include <vector>
#include <chrono>
#include <omp.h>

std::vector<std::vector<double>> generateMatrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = static_cast<double>(i + j) / 100.0;
        }
    }
    return matrix;
}

std::vector<std::vector<double>> multiplyMatrices(const std::vector<std::vector<double>>& A,
                                                  const std::vector<std::vector<double>>& B) {
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();
    
    std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            double sum = 0.0;
            for (int k = 0; k < colsA; ++k) {
                sum += A[i][k] * B[k][j];
            }
            result[i][j] = sum;
        }
    }
    
    return result;
}

void printMatrixStats(const std::vector<std::vector<double>>& matrix) {
    double sum = 0.0;
    double minVal = matrix[0][0];
    double maxVal = matrix[0][0];
    
    #pragma omp parallel for reduction(+:sum) reduction(min:minVal) reduction(max:maxVal) collapse(2)
    for (size_t i = 0; i < matrix.size(); ++i) {
        for (size_t j = 0; j < matrix[i].size(); ++j) {
            double val = matrix[i][j];
            sum += val;
            if (val < minVal) minVal = val;
            if (val > maxVal) maxVal = val;
        }
    }
    
    std::cout << "Matrix statistics:" << std::endl;
    std::cout << "  Sum: " << sum << std::endl;
    std::cout << "  Min: " << minVal << std::endl;
    std::cout << "  Max: " << maxVal << std::endl;
    std::cout << "  Avg: " << sum / (matrix.size() * matrix[0].size()) << std::endl;
}

int main() {
    const int SIZE = 500;
    
    auto start = std::chrono::high_resolution_clock::now();
    auto matrixA = generateMatrix(SIZE, SIZE);
    auto matrixB = generateMatrix(SIZE, SIZE);
    auto genTime = std::chrono::high_resolution_clock::now();
    
    std::cout << "Generated two " << SIZE << "x" << SIZE << " matrices" << std::endl;
    
    auto result = multiplyMatrices(matrixA, matrixB);
    auto multTime = std::chrono::high_resolution_clock::now();
    
    auto genDuration = std::chrono::duration_cast<std::chrono::milliseconds>(genTime - start);
    auto multDuration = std::chrono::duration_cast<std::chrono::milliseconds>(multTime - genTime);
    
    std::cout << "Matrix generation time: " << genDuration.count() << " ms" << std::endl;
    std::cout << "Matrix multiplication time: " << multDuration.count() << " ms" << std::endl;
    
    printMatrixStats(result);
    
    return 0;
}