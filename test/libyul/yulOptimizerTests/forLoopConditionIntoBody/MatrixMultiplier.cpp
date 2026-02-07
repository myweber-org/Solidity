
#include <iostream>
#include <vector>
#include <random>
#include <omp.h>
#include <chrono>

class Matrix {
private:
    std::vector<std::vector<double>> data;
    int rows;
    int cols;

public:
    Matrix(int r, int c) : rows(r), cols(c), data(r, std::vector<double>(c, 0.0)) {}

    void randomInitialize() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(1.0, 10.0);
        
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                data[i][j] = dis(gen);
            }
        }
    }

    void print() const {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                std::cout << data[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }

    Matrix multiplyParallel(const Matrix& other) const {
        if (cols != other.rows) {
            throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
        }

        Matrix result(rows, other.cols);
        
        #pragma omp parallel for collapse(2)
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < other.cols; ++j) {
                double sum = 0.0;
                for (int k = 0; k < cols; ++k) {
                    sum += data[i][k] * other.data[k][j];
                }
                result.data[i][j] = sum;
            }
        }
        
        return result;
    }

    Matrix multiplySequential(const Matrix& other) const {
        if (cols != other.rows) {
            throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
        }

        Matrix result(rows, other.cols);
        
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < other.cols; ++j) {
                double sum = 0.0;
                for (int k = 0; k < cols; ++k) {
                    sum += data[i][k] * other.data[k][j];
                }
                result.data[i][j] = sum;
            }
        }
        
        return result;
    }
};

int main() {
    const int SIZE = 500;
    
    Matrix A(SIZE, SIZE);
    Matrix B(SIZE, SIZE);
    
    A.randomInitialize();
    B.randomInitialize();
    
    auto start = std::chrono::high_resolution_clock::now();
    Matrix C_seq = A.multiplySequential(B);
    auto end = std::chrono::high_resolution_clock::now();
    auto seq_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    start = std::chrono::high_resolution_clock::now();
    Matrix C_par = A.multiplyParallel(B);
    end = std::chrono::high_resolution_clock::now();
    auto par_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Sequential multiplication time: " << seq_duration.count() << " ms" << std::endl;
    std::cout << "Parallel multiplication time: " << par_duration.count() << " ms" << std::endl;
    std::cout << "Speedup factor: " << static_cast<double>(seq_duration.count()) / par_duration.count() << std::endl;
    
    return 0;
}