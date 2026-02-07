
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

using namespace std;

vector<vector<int>> generateRandomMatrix(int rows, int cols) {
    vector<vector<int>> matrix(rows, vector<int>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = rand() % 100;
        }
    }
    return matrix;
}

vector<vector<int>> multiplyMatricesParallel(const vector<vector<int>>& A,
                                            const vector<vector<int>>& B) {
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();
    
    vector<vector<int>> result(rowsA, vector<int>(colsB, 0));
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            int sum = 0;
            for (int k = 0; k < colsA; ++k) {
                sum += A[i][k] * B[k][j];
            }
            result[i][j] = sum;
        }
    }
    
    return result;
}

void printMatrix(const vector<vector<int>>& matrix) {
    for (const auto& row : matrix) {
        for (int val : row) {
            cout << val << "\t";
        }
        cout << endl;
    }
}

int main() {
    srand(time(0));
    
    const int N = 500;
    const int M = 500;
    const int P = 500;
    
    cout << "Generating random matrices..." << endl;
    auto matrixA = generateRandomMatrix(N, M);
    auto matrixB = generateRandomMatrix(M, P);
    
    cout << "Performing parallel matrix multiplication..." << endl;
    double startTime = omp_get_wtime();
    
    auto result = multiplyMatricesParallel(matrixA, matrixB);
    
    double endTime = omp_get_wtime();
    
    cout << "Multiplication completed in " << (endTime - startTime) 
         << " seconds" << endl;
    
    cout << "Result matrix dimensions: " << result.size() 
         << " x " << result[0].size() << endl;
    
    return 0;
}#include <iostream>
#include <vector>
#include <stdexcept>

std::vector<std::vector<int>> multiplyMatrices(const std::vector<std::vector<int>>& matrixA, const std::vector<std::vector<int>>& matrixB) {
    int rowsA = matrixA.size();
    if (rowsA == 0) throw std::invalid_argument("Matrix A is empty");
    int colsA = matrixA[0].size();
    int rowsB = matrixB.size();
    if (rowsB == 0) throw std::invalid_argument("Matrix B is empty");
    int colsB = matrixB[0].size();

    if (colsA != rowsB) {
        throw std::invalid_argument("Matrix dimensions mismatch for multiplication");
    }

    for (const auto& row : matrixA) {
        if (row.size() != static_cast<size_t>(colsA)) {
            throw std::invalid_argument("Matrix A has inconsistent row sizes");
        }
    }
    for (const auto& row : matrixB) {
        if (row.size() != static_cast<size_t>(colsB)) {
            throw std::invalid_argument("Matrix B has inconsistent row sizes");
        }
    }

    std::vector<std::vector<int>> result(rowsA, std::vector<int>(colsB, 0));

    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            for (int k = 0; k < colsA; ++k) {
                result[i][j] += matrixA[i][k] * matrixB[k][j];
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
    try {
        std::vector<std::vector<int>> A = {{1, 2, 3}, {4, 5, 6}};
        std::vector<std::vector<int>> B = {{7, 8}, {9, 10}, {11, 12}};

        std::vector<std::vector<int>> C = multiplyMatrices(A, B);

        std::cout << "Resultant matrix:" << std::endl;
        printMatrix(C);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
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

    void multiplyParallel(int numThreads = 4) {
        omp_set_num_threads(numThreads);
        
        #pragma omp parallel for collapse(2)
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

    void verifyResults(const std::vector<std::vector<double>>& sequentialResult) {
        double tolerance = 1e-9;
        bool correct = true;
        
        for (int i = 0; i < rowsA && correct; ++i) {
            for (int j = 0; j < colsB && correct; ++j) {
                if (std::abs(result[i][j] - sequentialResult[i][j]) > tolerance) {
                    correct = false;
                    std::cerr << "Mismatch at position (" << i << "," << j << "): "
                              << result[i][j] << " vs " << sequentialResult[i][j] << std::endl;
                }
            }
        }
        
        if (correct) {
            std::cout << "Parallel computation verified successfully" << std::endl;
        } else {
            std::cout << "Parallel computation contains errors" << std::endl;
        }
    }

    void benchmarkMultiplication() {
        std::vector<std::vector<double>> sequentialResult = result;
        
        clock_t start = clock();
        multiplySequential();
        clock_t end = clock();
        double sequentialTime = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        sequentialResult = result;
        
        start = clock();
        multiplyParallel();
        end = clock();
        double parallelTime = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        
        std::cout << "Sequential time: " << sequentialTime << " seconds" << std::endl;
        std::cout << "Parallel time: " << parallelTime << " seconds" << std::endl;
        std::cout << "Speedup: " << sequentialTime / parallelTime << "x" << std::endl;
        
        verifyResults(sequentialResult);
    }

    void displayMatrix(const std::vector<std::vector<double>>& matrix, int maxRows = 5, int maxCols = 5) {
        int displayRows = std::min(maxRows, static_cast<int>(matrix.size()));
        int displayCols = (matrix.empty()) ? 0 : std::min(maxCols, static_cast<int>(matrix[0].size()));
        
        std::cout << "Matrix preview (first " << displayRows << "x" << displayCols << " elements):" << std::endl;
        for (int i = 0; i < displayRows; ++i) {
            for (int j = 0; j < displayCols; ++j) {
                std::cout << matrix[i][j] << "\t";
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    try {
        const int ROWS_A = 500;
        const int COLS_A = 500;
        const int ROWS_B = 500;
        const int COLS_B = 500;
        
        std::cout << "Initializing matrices for multiplication: " 
                  << ROWS_A << "x" << COLS_A << " * " 
                  << ROWS_B << "x" << COLS_B << std::endl;
        
        ParallelMatrixMultiplier multiplier(ROWS_A, COLS_A, ROWS_B, COLS_B);
        
        std::cout << "\nRunning benchmark..." << std::endl;
        multiplier.benchmarkMultiplication();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}