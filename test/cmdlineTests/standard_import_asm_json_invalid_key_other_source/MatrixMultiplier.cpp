
#include <iostream>
#include <vector>

class MatrixMultiplier {
public:
    static std::vector<std::vector<int>> multiply(const std::vector<std::vector<int>>& A,
                                                  const std::vector<std::vector<int>>& B) {
        int rowsA = A.size();
        int colsA = A[0].size();
        int rowsB = B.size();
        int colsB = B[0].size();

        if (colsA != rowsB) {
            throw std::invalid_argument("Matrix dimensions are not compatible for multiplication");
        }

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

    static void printMatrix(const std::vector<std::vector<int>>& matrix) {
        for (const auto& row : matrix) {
            for (int val : row) {
                std::cout << val << " ";
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    std::vector<std::vector<int>> A = {{1, 2, 3}, {4, 5, 6}};
    std::vector<std::vector<int>> B = {{7, 8}, {9, 10}, {11, 12}};

    try {
        std::vector<std::vector<int>> C = MatrixMultiplier::multiply(A, B);
        std::cout << "Matrix A:" << std::endl;
        MatrixMultiplier::printMatrix(A);
        std::cout << "Matrix B:" << std::endl;
        MatrixMultiplier::printMatrix(B);
        std::cout << "Result matrix C:" << std::endl;
        MatrixMultiplier::printMatrix(C);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

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
    std::vector<std::vector<double>> result;
    size_t rowsA, colsA, rowsB, colsB;

    void initializeRandomMatrix(std::vector<std::vector<double>>& matrix, size_t rows, size_t cols) {
        matrix.resize(rows, std::vector<double>(cols));
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                matrix[i][j] = static_cast<double>(rand()) / RAND_MAX * 100.0;
            }
        }
    }

public:
    ParallelMatrixMultiplier(size_t rA, size_t cA, size_t rB, size_t cB) 
        : rowsA(rA), colsA(cA), rowsB(rB), colsB(cB) {
        if (colsA != rowsB) {
            throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
        }
        
        srand(static_cast<unsigned>(time(nullptr)));
        initializeRandomMatrix(matrixA, rowsA, colsA);
        initializeRandomMatrix(matrixB, rowsB, colsB);
        result.resize(rowsA, std::vector<double>(colsB, 0.0));
    }

    void multiplySequential() {
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                result[i][j] = sum;
            }
        }
    }

    void multiplyParallel() {
        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                result[i][j] = sum;
            }
        }
    }

    void displayMatrix(const std::vector<std::vector<double>>& matrix, size_t maxRows = 5, size_t maxCols = 5) const {
        size_t displayRows = std::min(maxRows, matrix.size());
        size_t displayCols = (matrix.empty()) ? 0 : std::min(maxCols, matrix[0].size());
        
        std::cout << "Matrix preview (first " << displayRows << "x" << displayCols << " elements):\n";
        for (size_t i = 0; i < displayRows; ++i) {
            for (size_t j = 0; j < displayCols; ++j) {
                std::cout << matrix[i][j] << "\t";
            }
            std::cout << "\n";
        }
    }

    void benchmarkMultiplication() {
        double startTime, endTime;
        
        std::cout << "Benchmarking matrix multiplication (" << rowsA << "x" << colsA << " * " << rowsB << "x" << colsB << ")\n";
        
        startTime = omp_get_wtime();
        multiplySequential();
        endTime = omp_get_wtime();
        std::cout << "Sequential execution time: " << (endTime - startTime) << " seconds\n";
        
        startTime = omp_get_wtime();
        multiplyParallel();
        endTime = omp_get_wtime();
        std::cout << "Parallel execution time: " << (endTime - startTime) << " seconds\n";
        
        std::cout << "Speedup factor: " << ((endTime - startTime) > 0 ? 
                  (1.0 / (endTime - startTime)) * (rowsA * colsB * colsA) / 1e9 : 0) << " GFLOPs\n";
    }

    bool verifyResult(const std::vector<std::vector<double>>& reference) const {
        if (result.size() != reference.size() || result[0].size() != reference[0].size()) {
            return false;
        }
        
        const double epsilon = 1e-9;
        for (size_t i = 0; i < result.size(); ++i) {
            for (size_t j = 0; j < result[0].size(); ++j) {
                if (std::abs(result[i][j] - reference[i][j]) > epsilon) {
                    return false;
                }
            }
        }
        return true;
    }
};

int main() {
    try {
        const size_t SIZE = 512;
        ParallelMatrixMultiplier multiplier(SIZE, SIZE, SIZE, SIZE);
        
        multiplier.benchmarkMultiplication();
        
        std::cout << "\nMatrix A: ";
        // multiplier.displayMatrix(multiplier.matrixA);
        
        std::cout << "\nComputation completed successfully.\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}