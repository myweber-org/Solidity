
#include <iostream>
#include <vector>

using namespace std;

vector<vector<double>> multiplyMatrices(const vector<vector<double>>& A, const vector<vector<double>>& B) {
    int rowsA = A.size();
    int colsA = A[0].size();
    int rowsB = B.size();
    int colsB = B[0].size();

    if (colsA != rowsB) {
        throw invalid_argument("Matrices dimensions do not match for multiplication");
    }

    vector<vector<double>> result(rowsA, vector<double>(colsB, 0.0));

    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            for (int k = 0; k < colsA; ++k) {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    return result;
}

vector<vector<double>> transposeMatrix(const vector<vector<double>>& matrix) {
    int rows = matrix.size();
    int cols = matrix[0].size();

    vector<vector<double>> transposed(cols, vector<double>(rows));

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            transposed[j][i] = matrix[i][j];
        }
    }

    return transposed;
}

void printMatrix(const vector<vector<double>>& matrix) {
    for (const auto& row : matrix) {
        for (double val : row) {
            cout << val << " ";
        }
        cout << endl;
    }
}

int main() {
    vector<vector<double>> A = {{1, 2, 3}, {4, 5, 6}};
    vector<vector<double>> B = {{7, 8}, {9, 10}, {11, 12}};

    try {
        vector<vector<double>> C = multiplyMatrices(A, B);
        cout << "Matrix A:" << endl;
        printMatrix(A);
        cout << "\nMatrix B:" << endl;
        printMatrix(B);
        cout << "\nResult of A * B:" << endl;
        printMatrix(C);

        vector<vector<double>> AT = transposeMatrix(A);
        cout << "\nTranspose of A:" << endl;
        printMatrix(AT);
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}