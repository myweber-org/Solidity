
#include <iostream>
#include <vector>
#include <future>
#include <algorithm>
#include <numeric>
#include <random>
#include <chrono>

class DataProcessor {
private:
    std::vector<int> data;

    int processChunk(const std::vector<int>& chunk) {
        return std::accumulate(chunk.begin(), chunk.end(), 0);
    }

public:
    DataProcessor(size_t size) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1, 100);
        
        data.resize(size);
        std::generate(data.begin(), data.end(), [&]() { return dis(gen); });
    }

    int processSequential() {
        auto start = std::chrono::high_resolution_clock::now();
        int result = processChunk(data);
        auto end = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double> elapsed = end - start;
        std::cout << "Sequential processing time: " << elapsed.count() << " seconds\n";
        return result;
    }

    int processParallel(size_t numChunks) {
        auto start = std::chrono::high_resolution_clock::now();
        
        size_t chunkSize = data.size() / numChunks;
        std::vector<std::future<int>> futures;
        
        for (size_t i = 0; i < numChunks; ++i) {
            size_t startIdx = i * chunkSize;
            size_t endIdx = (i == numChunks - 1) ? data.size() : startIdx + chunkSize;
            
            std::vector<int> chunk(data.begin() + startIdx, data.begin() + endIdx);
            futures.push_back(std::async(std::launch::async, &DataProcessor::processChunk, this, chunk));
        }
        
        int total = 0;
        for (auto& future : futures) {
            total += future.get();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        std::cout << "Parallel processing time: " << elapsed.count() << " seconds\n";
        
        return total;
    }

    void printStats() const {
        std::cout << "Data size: " << data.size() << " elements\n";
        std::cout << "Data range: [" << *std::min_element(data.begin(), data.end()) 
                  << ", " << *std::max_element(data.begin(), data.end()) << "]\n";
    }
};

int main() {
    const size_t DATA_SIZE = 10000000;
    const size_t NUM_CHUNKS = 4;
    
    DataProcessor processor(DATA_SIZE);
    processor.printStats();
    
    std::cout << "\nProcessing data...\n";
    int seqResult = processor.processSequential();
    int parResult = processor.processParallel(NUM_CHUNKS);
    
    std::cout << "\nSequential result: " << seqResult << "\n";
    std::cout << "Parallel result: " << parResult << "\n";
    std::cout << "Results match: " << (seqResult == parResult ? "YES" : "NO") << "\n";
    
    return 0;
}