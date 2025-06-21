#include <iostream>
#include <unordered_map>
#include <vector>
#include <deque>
#include <mutex>
#include <chrono>
#include <memory>
#include <thread>

class SlidingLog {
private:
    std::deque<std::chrono::steady_clock::time_point> timestamps;
    int limit;
    std::chrono::milliseconds windowSize;
    std::mutex mtx;

    void cleanup() {
        auto now = std::chrono::steady_clock::now();
        while (!timestamps.empty() && now - timestamps.front() >= windowSize) {
            timestamps.pop_front();
        }
    }

public:
    SlidingLog(int limit, std::chrono::milliseconds windowSize)
        : limit(limit), windowSize(windowSize) {}

    bool allowRequest() {
        std::lock_guard<std::mutex> lock(mtx);
        cleanup();

        if ((int)timestamps.size() < limit) {
            timestamps.push_back(std::chrono::steady_clock::now());
            return true;
        }
        return false;
    }
};

class SlidingWindowLogRateLimiter {
private:
    std::unordered_map<std::string, std::shared_ptr<SlidingLog>> userLogs;
    std::mutex mapMutex;
    int limit;
    std::chrono::milliseconds windowSize;

public:
    SlidingWindowLogRateLimiter(int limit, std::chrono::milliseconds windowSize)
        : limit(limit), windowSize(windowSize) {}

    bool allowRequest(const std::string& userId) {
        std::shared_ptr<SlidingLog> log;

        {
            std::lock_guard<std::mutex> lock(mapMutex);
            auto it = userLogs.find(userId);
            if (it == userLogs.end()) {
                log = std::make_shared<SlidingLog>(limit, windowSize);
                userLogs[userId] = log;
            } else {
                log = it->second;
            }
        }

        return log->allowRequest();
    }
};

void testSlidingWindow(SlidingWindowLogRateLimiter& limiter, const std::string& userId, int threadId) {
    if (limiter.allowRequest(userId)) {
        std::cout << "[SlidingWindow] Thread " << threadId << ": Allowed\n";
    } else {
        std::cout << "[SlidingWindow] Thread " << threadId << ": Denied\n";
    }
}

int main() {
    const std::string userId = "User_Arkaza";
    const int numThreads = 15;

    // === Sliding Window: max 5 requests in a 2-second window
    SlidingWindowLogRateLimiter slidingLimiter(5, std::chrono::milliseconds(2000));

    std::cout << "\n=== Testing Sliding Window Log Rate Limiter ===\n";
    std::vector<std::thread> slidingThreads;
    for (int i = 0; i < numThreads; ++i) {
        slidingThreads.emplace_back(testSlidingWindow, std::ref(slidingLimiter), userId, i);
        std::this_thread::sleep_for(std::chrono::milliseconds(300)); // delay to observe sliding effect
    }

    for (auto& t : slidingThreads) {
        t.join();
    }

    return 0;
}