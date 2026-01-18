
#include <iostream>
#include <vector>

std::vector<std::vector<int>> multiplyMatrices(const std::vector<std::vector<int>>& A,
                                               const std::vector<std::vector<int>>& B) {
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();

    std::vector<std::vector<int>> result(rowsA, std::vector<int>(colsB, 0));

    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            for (int k = 0; k < colsA; ++k) {
                result[i][j] += A[i][k] * B[k][j];
            }
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
    std::vector<std::vector<int>> matrixA = {{1, 2, 3},
                                             {4, 5, 6}};

    std::vector<std::vector<int>> matrixB = {{7, 8},
                                             {9, 10},
                                             {11, 12}};

    std::vector<std::vector<int>> product = multiplyMatrices(matrixA, matrixB);

    std::cout << "Result of matrix multiplication:" << std::endl;
    printMatrix(product);

    return 0;
}
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

class ParallelMatrixMultiplier {
private:
    std::vector<std::vector<double>> matrixA;
    std::vector<std::vector<double>> matrixB;
    std::vector<std::vector<double>> resultMatrix;
    int rowsA, colsA, rowsB, colsB;

public:
    ParallelMatrixMultiplier(int rA, int cA, int rB, int cB) 
        : rowsA(rA), colsA(cA), rowsB(rB), colsB(cB) {
        
        if (colsA != rowsB) {
            throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
        }

        matrixA.resize(rowsA, std::vector<double>(colsA));
        matrixB.resize(rowsB, std::vector<double>(colsB));
        resultMatrix.resize(rowsA, std::vector<double>(colsB, 0.0));

        initializeMatrices();
    }

    void initializeMatrices() {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        
        #pragma omp parallel for collapse(2)
        for (int i = 0; i < rowsA; ++i) {
            for (int j = 0; j < colsA; ++j) {
                matrixA[i][j] = static_cast<double>(std::rand()) / RAND_MAX * 100.0;
            }
        }

        #pragma omp parallel for collapse(2)
        for (int i = 0; i < rowsB; ++i) {
            for (int j = 0; j < colsB; ++j) {
                matrixB[i][j] = static_cast<double>(std::rand()) / RAND_MAX * 100.0;
            }
        }
    }

    void multiplyMatrices() {
        #pragma omp parallel for collapse(2)
        for (int i = 0; i < rowsA; ++i) {
            for (int j = 0; j < colsB; ++j) {
                double sum = 0.0;
                #pragma omp simd reduction(+:sum)
                for (int k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                resultMatrix[i][j] = sum;
            }
        }
    }

    void displayResult(int maxRows = 5, int maxCols = 5) {
        std::cout << "Result Matrix (first " << maxRows << "x" << maxCols << " elements):\n";
        for (int i = 0; i < std::min(rowsA, maxRows); ++i) {
            for (int j = 0; j < std::min(colsB, maxCols); ++j) {
                std::cout << resultMatrix[i][j] << "\t";
            }
            std::cout << "\n";
        }
    }

    double verifyMultiplication() {
        double totalSum = 0.0;
        #pragma omp parallel for reduction(+:totalSum) collapse(2)
        for (int i = 0; i < rowsA; ++i) {
            for (int j = 0; j < colsB; ++j) {
                totalSum += resultMatrix[i][j];
            }
        }
        return totalSum;
    }
};

int main() {
    const int SIZE = 512;
    
    try {
        ParallelMatrixMultiplier multiplier(SIZE, SIZE, SIZE, SIZE);
        
        double startTime = omp_get_wtime();
        multiplier.multiplyMatrices();
        double endTime = omp_get_wtime();
        
        std::cout << "Matrix multiplication completed for " << SIZE << "x" << SIZE << " matrices\n";
        std::cout << "Execution time: " << (endTime - startTime) << " seconds\n";
        
        double checksum = multiplier.verifyMultiplication();
        std::cout << "Verification checksum: " << checksum << "\n";
        
        if (SIZE <= 10) {
            multiplier.displayResult();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}