
#include <vector>
#include <deque>
#include <stdexcept>

class MovingAverageCalculator {
public:
    explicit MovingAverageCalculator(size_t windowSize) : windowSize_(windowSize) {
        if (windowSize == 0) {
            throw std::invalid_argument("Window size must be greater than zero");
        }
    }

    void addValue(double value) {
        values_.push_back(value);
        sum_ += value;

        if (values_.size() > windowSize_) {
            sum_ -= values_.front();
            values_.pop_front();
        }
    }

    double getAverage() const {
        if (values_.empty()) {
            return 0.0;
        }
        return sum_ / values_.size();
    }

    bool isWindowFull() const {
        return values_.size() == windowSize_;
    }

    void clear() {
        values_.clear();
        sum_ = 0.0;
    }

    size_t getCurrentCount() const {
        return values_.size();
    }

private:
    std::deque<double> values_;
    double sum_ = 0.0;
    size_t windowSize_;
};