
#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>
#include <random>

class ThreadSafeQueue {
private:
    std::queue<int> data_queue;
    mutable std::mutex queue_mutex;
    std::condition_variable queue_cond;
    bool stop_flag = false;
    const size_t max_size = 100;

public:
    void push(int value) {
        std::unique_lock<std::mutex> lock(queue_mutex);
        queue_cond.wait(lock, [this]() { return data_queue.size() < max_size || stop_flag; });
        if (stop_flag) return;
        data_queue.push(value);
        lock.unlock();
        queue_cond.notify_one();
    }

    bool try_pop(int& value) {
        std::unique_lock<std::mutex> lock(queue_mutex);
        if (data_queue.empty() || stop_flag) return false;
        value = data_queue.front();
        data_queue.pop();
        lock.unlock();
        queue_cond.notify_one();
        return true;
    }

    void wait_and_pop(int& value) {
        std::unique_lock<std::mutex> lock(queue_mutex);
        queue_cond.wait(lock, [this]() { return !data_queue.empty() || stop_flag; });
        if (stop_flag) return;
        value = data_queue.front();
        data_queue.pop();
        lock.unlock();
        queue_cond.notify_one();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(queue_mutex);
        return data_queue.size();
    }

    void stop() {
        std::lock_guard<std::mutex> lock(queue_mutex);
        stop_flag = true;
        queue_cond.notify_all();
    }

    bool is_stopped() const {
        std::lock_guard<std::mutex> lock(queue_mutex);
        return stop_flag;
    }
};

void producer(ThreadSafeQueue& queue, int id, int items_to_produce) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1000);

    for (int i = 0; i < items_to_produce; ++i) {
        int value = dis(gen);
        queue.push(value);
        std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen) % 50));
    }
}

void consumer(ThreadSafeQueue& queue, int id) {
    int value;
    while (!queue.is_stopped()) {
        if (queue.try_pop(value)) {
            std::cout << "Consumer " << id << " processed: " << value << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
    while (queue.try_pop(value)) {
        std::cout << "Consumer " << id << " processed remaining: " << value << std::endl;
    }
}

int main() {
    ThreadSafeQueue queue;
    const int num_producers = 3;
    const int num_consumers = 2;
    const int items_per_producer = 20;

    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    for (int i = 0; i < num_producers; ++i) {
        producers.emplace_back(producer, std::ref(queue), i + 1, items_per_producer);
    }

    for (int i = 0; i < num_consumers; ++i) {
        consumers.emplace_back(consumer, std::ref(queue), i + 1);
    }

    for (auto& p : producers) {
        p.join();
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    queue.stop();

    for (auto& c : consumers) {
        c.join();
    }

    std::cout << "Final queue size: " << queue.size() << std::endl;
    std::cout << "Processing completed." << std::endl;

    return 0;
}