
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <omp.h>

class MatrixMultiplier {
public:
    static std::vector<std::vector<double>> generateRandomMatrix(int rows, int cols) {
        std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 10.0);
        
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                matrix[i][j] = dis(gen);
            }
        }
        return matrix;
    }

    static std::vector<std::vector<double>> multiplyParallel(
        const std::vector<std::vector<double>>& A,
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

    static std::vector<std::vector<double>> multiplySequential(
        const std::vector<std::vector<double>>& A,
        const std::vector<std::vector<double>>& B) {
        
        int rowsA = A.size();
        int colsA = A[0].size();
        int colsB = B[0].size();
        
        std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));
        
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

    static bool verifyResult(
        const std::vector<std::vector<double>>& result1,
        const std::vector<std::vector<double>>& result2,
        double tolerance = 1e-6) {
        
        if (result1.size() != result2.size() || result1[0].size() != result2[0].size()) {
            return false;
        }
        
        for (size_t i = 0; i < result1.size(); ++i) {
            for (size_t j = 0; j < result1[0].size(); ++j) {
                if (std::abs(result1[i][j] - result2[i][j]) > tolerance) {
                    return false;
                }
            }
        }
        return true;
    }
};

int main() {
    const int N = 500;
    
    std::cout << "Generating random matrices of size " << N << "x" << N << "..." << std::endl;
    auto A = MatrixMultiplier::generateRandomMatrix(N, N);
    auto B = MatrixMultiplier::generateRandomMatrix(N, N);
    
    std::cout << "Performing sequential multiplication..." << std::endl;
    auto startSeq = std::chrono::high_resolution_clock::now();
    auto resultSeq = MatrixMultiplier::multiplySequential(A, B);
    auto endSeq = std::chrono::high_resolution_clock::now();
    auto durationSeq = std::chrono::duration_cast<std::chrono::milliseconds>(endSeq - startSeq);
    
    std::cout << "Performing parallel multiplication..." << std::endl;
    auto startPar = std::chrono::high_resolution_clock::now();
    auto resultPar = MatrixMultiplier::multiplyParallel(A, B);
    auto endPar = std::chrono::high_resolution_clock::now();
    auto durationPar = std::chrono::duration_cast<std::chrono::milliseconds>(endPar - startPar);
    
    std::cout << "\nPerformance Results:" << std::endl;
    std::cout << "Sequential time: " << durationSeq.count() << " ms" << std::endl;
    std::cout << "Parallel time: " << durationPar.count() << " ms" << std::endl;
    std::cout << "Speedup: " << static_cast<double>(durationSeq.count()) / durationPar.count() << "x" << std::endl;
    
    std::cout << "\nVerifying results..." << std::endl;
    if (MatrixMultiplier::verifyResult(resultSeq, resultPar)) {
        std::cout << "Results match!" << std::endl;
    } else {
        std::cout << "Results do not match!" << std::endl;
    }
    
    return 0;
}