#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <vector>

class DataProcessor {
private:
    std::queue<int> dataQueue;
    std::mutex queueMutex;
    std::condition_variable queueCondVar;
    bool stopProcessing = false;
    const size_t maxQueueSize = 10;

public:
    void producer(int id) {
        for (int i = 0; i < 20; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCondVar.wait(lock, [this]() { 
                return dataQueue.size() < maxQueueSize || stopProcessing; 
            });
            
            if (stopProcessing) {
                break;
            }
            
            int data = id * 100 + i;
            dataQueue.push(data);
            std::cout << "Producer " << id << " produced: " << data << std::endl;
            
            lock.unlock();
            queueCondVar.notify_all();
        }
    }

    void consumer(int id) {
        while (true) {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCondVar.wait(lock, [this]() { 
                return !dataQueue.empty() || stopProcessing; 
            });
            
            if (stopProcessing && dataQueue.empty()) {
                break;
            }
            
            if (!dataQueue.empty()) {
                int data = dataQueue.front();
                dataQueue.pop();
                lock.unlock();
                
                std::cout << "Consumer " << id << " processed: " << data << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }

    void run() {
        std::vector<std::thread> producers;
        std::vector<std::thread> consumers;
        
        for (int i = 0; i < 3; ++i) {
            producers.emplace_back(&DataProcessor::producer, this, i);
        }
        
        for (int i = 0; i < 2; ++i) {
            consumers.emplace_back(&DataProcessor::consumer, this, i);
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(3));
        
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            stopProcessing = true;
        }
        queueCondVar.notify_all();
        
        for (auto& producer : producers) {
            producer.join();
        }
        
        for (auto& consumer : consumers) {
            consumer.join();
        }
        
        std::cout << "Data processing completed. Remaining queue size: " 
                  << dataQueue.size() << std::endl;
    }
};

int main() {
    DataProcessor processor;
    processor.run();
    return 0;
}