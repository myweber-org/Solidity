
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
        #pragma omp parallel for collapse(2) schedule(dynamic)
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

    void verifyMultiplication() {
        std::vector<std::vector<double>> sequentialResult(rowsA, std::vector<double>(colsB, 0.0));
        
        for (size_t i = 0; i < rowsA; ++i) {
            for (size_t j = 0; j < colsB; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < colsA; ++k) {
                    sum += matrixA[i][k] * matrixB[k][j];
                }
                sequentialResult[i][j] = sum;
            }
        }

        bool correct = true;
        const double epsilon = 1e-9;
        for (size_t i = 0; i < rowsA && correct; ++i) {
            for (size_t j = 0; j < colsB && correct; ++j) {
                if (std::abs(result[i][j] - sequentialResult[i][j]) > epsilon) {
                    correct = false;
                }
            }
        }

        std::cout << "Verification: " << (correct ? "PASSED" : "FAILED") << std::endl;
    }

    void printMatrix(const std::vector<std::vector<double>>& matrix, size_t maxRows = 3, size_t maxCols = 3) {
        size_t printRows = std::min(maxRows, matrix.size());
        for (size_t i = 0; i < printRows; ++i) {
            size_t printCols = std::min(maxCols, matrix[i].size());
            for (size_t j = 0; j < printCols; ++j) {
                std::cout << matrix[i][j] << " ";
            }
            if (printCols < matrix[i].size()) std::cout << "...";
            std::cout << std::endl;
        }
        if (printRows < matrix.size()) std::cout << "..." << std::endl;
    }

    void benchmark() {
        clock_t start, end;
        
        std::cout << "Matrix A dimensions: " << rowsA << "x" << colsA << std::endl;
        std::cout << "Matrix B dimensions: " << rowsB << "x" << colsB << std::endl;
        
        start = clock();
        multiplySequential();
        end = clock();
        double sequentialTime = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        
        start = clock();
        multiplyParallel();
        end = clock();
        double parallelTime = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        
        std::cout << "Sequential time: " << sequentialTime << " seconds" << std::endl;
        std::cout << "Parallel time: " << parallelTime << " seconds" << std::endl;
        std::cout << "Speedup: " << sequentialTime / parallelTime << "x" << std::endl;
        
        verifyMultiplication();
    }
};

int main() {
    try {
        const size_t rowsA = 500;
        const size_t colsA = 500;
        const size_t rowsB = 500;
        const size_t colsB = 500;
        
        ParallelMatrixMultiplier multiplier(rowsA, colsA, rowsB, colsB);
        multiplier.benchmark();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

void initializeMatrix(std::vector<std::vector<double>>& matrix, int size) {
    matrix.resize(size, std::vector<double>(size));
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX;
        }
    }
}

void multiplyMatrices(const std::vector<std::vector<double>>& A,
                      const std::vector<std::vector<double>>& B,
                      std::vector<std::vector<double>>& C,
                      int size) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            double sum = 0.0;
            for (int k = 0; k < size; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

int main() {
    const int MATRIX_SIZE = 500;
    std::vector<std::vector<double>> A, B, C;

    srand(static_cast<unsigned>(time(nullptr)));

    initializeMatrix(A, MATRIX_SIZE);
    initializeMatrix(B, MATRIX_SIZE);
    C.resize(MATRIX_SIZE, std::vector<double>(MATRIX_SIZE));

    double start_time = omp_get_wtime();
    multiplyMatrices(A, B, C, MATRIX_SIZE);
    double end_time = omp_get_wtime();

    std::cout << "Matrix multiplication completed for size " << MATRIX_SIZE << std::endl;
    std::cout << "Execution time: " << (end_time - start_time) << " seconds" << std::endl;

    return 0;
}
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <omp.h>

class Matrix {
private:
    std::vector<std::vector<double>> data;
    size_t rows;
    size_t cols;

public:
    Matrix(size_t r, size_t c) : rows(r), cols(c) {
        data.resize(rows, std::vector<double>(cols, 0.0));
    }

    void randomFill() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 10.0);

        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                data[i][j] = dis(gen);
            }
        }
    }

    Matrix multiply(const Matrix& other) const {
        if (cols != other.rows) {
            throw std::invalid_argument("Matrix dimensions mismatch for multiplication");
        }

        Matrix result(rows, other.cols);

        #pragma omp parallel for collapse(2)
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < other.cols; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < cols; ++k) {
                    sum += data[i][k] * other.data[k][j];
                }
                result.data[i][j] = sum;
            }
        }

        return result;
    }

    void display(size_t limit = 5) const {
        size_t displayRows = std::min(rows, limit);
        size_t displayCols = std::min(cols, limit);

        for (size_t i = 0; i < displayRows; ++i) {
            for (size_t j = 0; j < displayCols; ++j) {
                std::cout << data[i][j] << " ";
            }
            if (displayCols < cols) {
                std::cout << "...";
            }
            std::cout << std::endl;
        }
        if (displayRows < rows) {
            std::cout << "..." << std::endl;
        }
    }
};

int main() {
    const size_t N = 1000;
    
    std::cout << "Initializing matrices of size " << N << "x" << N << std::endl;
    
    Matrix A(N, N);
    Matrix B(N, N);
    
    A.randomFill();
    B.randomFill();
    
    std::cout << "Starting matrix multiplication..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    
    Matrix C = A.multiply(B);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Multiplication completed in " << duration.count() << " ms" << std::endl;
    
    std::cout << "\nFirst 5x5 elements of matrix A:" << std::endl;
    A.display();
    
    std::cout << "\nFirst 5x5 elements of matrix B:" << std::endl;
    B.display();
    
    std::cout << "\nFirst 5x5 elements of result matrix C:" << std::endl;
    C.display();
    
    return 0;
}