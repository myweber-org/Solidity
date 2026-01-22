#include <vector>
#include <algorithm>
#include <functional>

class DataProcessor {
public:
    template<typename T>
    static std::vector<T> filter(const std::vector<T>& data, std::function<bool(const T&)> predicate) {
        std::vector<T> result;
        std::copy_if(data.begin(), data.end(), std::back_inserter(result), predicate);
        return result;
    }

    template<typename T, typename U>
    static std::vector<U> transform(const std::vector<T>& data, std::function<U(const T&)> transformer) {
        std::vector<U> result;
        result.reserve(data.size());
        std::transform(data.begin(), data.end(), std::back_inserter(result), transformer);
        return result;
    }

    template<typename T>
    static T aggregate(const std::vector<T>& data, T initial, std::function<T(T, T)> accumulator) {
        return std::accumulate(data.begin(), data.end(), initial, accumulator);
    }
};