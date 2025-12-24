
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
    int size;

public:
    ParallelMatrixMultiplier(int n) : size(n) {
        matrixA.resize(n, std::vector<double>(n));
        matrixB.resize(n, std::vector<double>(n));
        result.resize(n, std::vector<double>(n, 0.0));
        initializeMatrices();
    }

    void initializeMatrices() {
        srand(static_cast<unsigned>(time(nullptr)));
        #pragma omp parallel for collapse(2)
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                matrixA[i][j] = static_cast<double>(rand()) / RAND_MAX;
                matrixB[i][j] = static_cast<double>(rand()) / RAND_MAX;
            }
        }
    }

    void multiply() {
        #pragma omp parallel for collapse(2)
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                double sum = 0.0;
                for (int k = 0; k < size; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                result[i][j] = sum;
            }
        }
    }

    void displayResult(int limit = 5) {
        std::cout << "First " << limit << "x" << limit << " elements of result matrix:" << std::endl;
        for (int i = 0; i < std::min(limit, size); ++i) {
            for (int j = 0; j < std::min(limit, size); ++j) {
                std::cout << result[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }

    double verifyMultiplication() {
        double checksum = 0.0;
        #pragma omp parallel for reduction(+:checksum) collapse(2)
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                checksum += result[i][j];
            }
        }
        return checksum;
    }
};

int main() {
    const int MATRIX_SIZE = 500;
    
    std::cout << "Initializing parallel matrix multiplier with size " 
              << MATRIX_SIZE << "x" << MATRIX_SIZE << std::endl;
    
    ParallelMatrixMultiplier multiplier(MATRIX_SIZE);
    
    double start_time = omp_get_wtime();
    multiplier.multiply();
    double end_time = omp_get_wtime();
    
    std::cout << "Parallel multiplication completed in " 
              << (end_time - start_time) << " seconds" << std::endl;
    
    multiplier.displayResult();
    
    double checksum = multiplier.verifyMultiplication();
    std::cout << "Matrix checksum: " << checksum << std::endl;
    
    return 0;
}
#include <iostream>
#include <vector>
#include <omp.h>
#include <cstdlib>
#include <ctime>

class ParallelMatrixMultiplier {
private:
    std::vector<std::vector<double>> matrixA;
    std::vector<std::vector<double>> matrixB;
    std::vector<std::vector<double>> result;
    int rowsA, colsA, rowsB, colsB;

    void initializeRandomMatrix(std::vector<std::vector<double>>& matrix, int rows, int cols) {
        matrix.resize(rows, std::vector<double>(cols));
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                matrix[i][j] = static_cast<double>(rand()) / RAND_MAX * 100.0;
            }
        }
    }

public:
    ParallelMatrixMultiplier(int rA, int cA, int rB, int cB) 
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
        for (int i = 0; i < rowsA; ++i) {
            for (int j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (int k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                result[i][j] = sum;
            }
        }
    }

    void multiplyParallel() {
        #pragma omp parallel for collapse(2) schedule(dynamic)
        for (int i = 0; i < rowsA; ++i) {
            for (int j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (int k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                result[i][j] = sum;
            }
        }
    }

    void displayResult(int limit = 5) const {
        int displayRows = std::min(limit, rowsA);
        int displayCols = std::min(limit, colsB);
        
        std::cout << "First " << displayRows << "x" << displayCols << " elements of result:\n";
        for (int i = 0; i < displayRows; ++i) {
            for (int j = 0; j < displayCols; ++j) {
                std::cout << result[i][j] << "\t";
            }
            std::cout << "\n";
        }
    }

    double verifyMultiplication() const {
        double totalSum = 0.0;
        for (const auto& row : result) {
            for (double val : row) {
                totalSum += val;
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
        multiplier.multiplySequential();
        double seqTime = omp_get_wtime() - startTime;
        
        std::cout << "Sequential multiplication time: " << seqTime << " seconds\n";
        
        startTime = omp_get_wtime();
        multiplier.multiplyParallel();
        double parTime = omp_get_wtime() - startTime;
        
        std::cout << "Parallel multiplication time: " << parTime << " seconds\n";
        std::cout << "Speedup factor: " << seqTime / parTime << "\n";
        
        multiplier.displayResult();
        std::cout << "Verification sum: " << multiplier.verifyMultiplication() << "\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}