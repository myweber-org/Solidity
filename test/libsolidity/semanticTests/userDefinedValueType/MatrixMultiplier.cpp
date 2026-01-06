
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

public:
    ParallelMatrixMultiplier(int rA, int cA, int rB, int cB) 
        : rowsA(rA), colsA(cA), rowsB(rB), colsB(cB) {
        
        if (colsA != rowsB) {
            throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
        }

        matrixA.resize(rowsA, std::vector<double>(colsA));
        matrixB.resize(rowsB, std::vector<double>(colsB));
        result.resize(rowsA, std::vector<double>(colsB, 0.0));

        initializeRandomMatrices();
    }

    void initializeRandomMatrices() {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        
        for (int i = 0; i < rowsA; ++i) {
            for (int j = 0; j < colsA; ++j) {
                matrixA[i][j] = static_cast<double>(std::rand()) / RAND_MAX * 100.0;
            }
        }

        for (int i = 0; i < rowsB; ++i) {
            for (int j = 0; j < colsB; ++j) {
                matrixB[i][j] = static_cast<double>(std::rand()) / RAND_MAX * 100.0;
            }
        }
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

    void displayMatrix(const std::vector<std::vector<double>>& mat, const std::string& name) {
        std::cout << name << " (" << mat.size() << "x" 
                  << (mat.empty() ? 0 : mat[0].size()) << "):\n";
        
        for (size_t i = 0; i < std::min(mat.size(), static_cast<size_t>(5)); ++i) {
            for (size_t j = 0; j < std::min(mat[i].size(), static_cast<size_t>(5)); ++j) {
                std::cout << mat[i][j] << "\t";
            }
            if (mat[i].size() > 5) std::cout << "...";
            std::cout << "\n";
        }
        if (mat.size() > 5) std::cout << "...\n";
        std::cout << std::endl;
    }

    void benchmarkMultiplication() {
        double start, end;
        
        std::cout << "Benchmarking matrix multiplication (" 
                  << rowsA << "x" << colsA << ") * (" 
                  << rowsB << "x" << colsB << ")...\n";

        start = omp_get_wtime();
        multiplySequential();
        end = omp_get_wtime();
        std::cout << "Sequential execution time: " << (end - start) << " seconds\n";

        start = omp_get_wtime();
        multiplyParallel();
        end = omp_get_wtime();
        std::cout << "Parallel execution time: " << (end - start) << " seconds\n";

        std::cout << "Speedup factor: " 
                  << ((end - start) > 0 ? (rowsA * colsB * colsA) / (end - start) / 1e6 : 0) 
                  << " MFLOPS\n";
    }

    bool verifyResult(const std::vector<std::vector<double>>& reference) {
        if (result.size() != reference.size() || 
            result[0].size() != reference[0].size()) {
            return false;
        }

        const double epsilon = 1e-10;
        for (size_t i = 0; i < result.size(); ++i) {
            for (size_t j = 0; j < result[i].size(); ++j) {
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
        const int rowsA = 500;
        const int colsA = 500;
        const int rowsB = 500;
        const int colsB = 500;

        ParallelMatrixMultiplier multiplier(rowsA, colsA, rowsB, colsB);
        
        multiplier.benchmarkMultiplication();

        std::cout << "\nFirst 5x5 elements of result matrix:\n";
        multiplier.displayMatrix(
            [&]() -> std::vector<std::vector<double>> {
                std::vector<std::vector<double>> sample(5, std::vector<double>(5));
                auto temp = multiplier;
                temp.multiplyParallel();
                return temp.result;
            }(), 
            "Result"
        );

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}