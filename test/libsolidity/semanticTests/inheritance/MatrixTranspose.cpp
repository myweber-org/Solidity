
#include <iostream>
#include <vector>

using namespace std;

vector<vector<int>> transposeMatrix(const vector<vector<int>>& matrix) {
    if (matrix.empty()) return {};
    
    int rows = matrix.size();
    int cols = matrix[0].size();
    
    vector<vector<int>> result(cols, vector<int>(rows));
    
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            result[j][i] = matrix[i][j];
        }
    }
    
    return result;
}

void printMatrix(const vector<vector<int>>& matrix) {
    for (const auto& row : matrix) {
        for (int val : row) {
            cout << val << " ";
        }
        cout << endl;
    }
}

int main() {
    vector<vector<int>> original = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };
    
    cout << "Original matrix:" << endl;
    printMatrix(original);
    
    vector<vector<int>> transposed = transposeMatrix(original);
    
    cout << "\nTransposed matrix:" << endl;
    printMatrix(transposed);
    
    return 0;
}