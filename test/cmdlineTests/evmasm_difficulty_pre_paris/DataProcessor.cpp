#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>
#include <random>

class DataProcessor {
private:
    std::queue<int> dataQueue;
    std::mutex queueMutex;
    std::condition_variable queueCondVar;
    bool processingComplete;
    const size_t maxQueueSize;

    void producerFunction(int id, int itemsToProduce) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1, 100);
        std::uniform_int_distribution<> sleepDis(10, 50);

        for (int i = 0; i < itemsToProduce; ++i) {
            int data = dis(gen);
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepDis(gen)));

            std::unique_lock<std::mutex> lock(queueMutex);
            queueCondVar.wait(lock, [this]() { 
                return dataQueue.size() < maxQueueSize || processingComplete; 
            });

            if (processingComplete) {
                break;
            }

            dataQueue.push(data);
            std::cout << "Producer " << id << " generated: " << data << std::endl;
            lock.unlock();
            queueCondVar.notify_all();
        }

        std::cout << "Producer " << id << " finished." << std::endl;
    }

    void consumerFunction(int id) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> sleepDis(20, 100);

        while (true) {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCondVar.wait(lock, [this]() { 
                return !dataQueue.empty() || processingComplete; 
            });

            if (processingComplete && dataQueue.empty()) {
                break;
            }

            if (!dataQueue.empty()) {
                int data = dataQueue.front();
                dataQueue.pop();
                lock.unlock();

                std::this_thread::sleep_for(std::chrono::milliseconds(sleepDis(gen)));
                int result = processData(data);
                std::cout << "Consumer " << id << " processed: " << data << " -> " << result << std::endl;

                queueCondVar.notify_all();
            }
        }

        std::cout << "Consumer " << id << " finished." << std::endl;
    }

    int processData(int value) {
        return value * value + 2 * value + 1;
    }

public:
    DataProcessor(size_t maxSize = 10) : maxQueueSize(maxSize), processingComplete(false) {}

    void runProcessing(int producerCount, int consumerCount, int itemsPerProducer) {
        std::vector<std::thread> producers;
        std::vector<std::thread> consumers;

        for (int i = 0; i < producerCount; ++i) {
            producers.emplace_back(&DataProcessor::producerFunction, this, i + 1, itemsPerProducer);
        }

        for (int i = 0; i < consumerCount; ++i) {
            consumers.emplace_back(&DataProcessor::consumerFunction, this, i + 1);
        }

        for (auto& producer : producers) {
            producer.join();
        }

        {
            std::lock_guard<std::mutex> lock(queueMutex);
            processingComplete = true;
        }
        queueCondVar.notify_all();

        for (auto& consumer : consumers) {
            consumer.join();
        }

        std::cout << "Data processing completed. Queue size: " << dataQueue.size() << std::endl;
    }
};

int main() {
    DataProcessor processor(15);
    processor.runProcessing(3, 2, 8);
    return 0;
}