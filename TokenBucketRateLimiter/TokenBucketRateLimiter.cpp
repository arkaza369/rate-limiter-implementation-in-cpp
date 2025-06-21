#include <iostream>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <memory>
#include <cstring>
#include <vector>
#include <thread>

using namespace std;

class TokenBucket {
private:
    int capacity;
    double tokens;
    double refillRatePerMs;
    std::chrono::steady_clock::time_point lastRefillTime;
    std::mutex mtx;

    void refillTokens() {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> elapsed = now - lastRefillTime;
        double tokensToAdd = elapsed.count() * refillRatePerMs;
        tokens = std::min((double)capacity, tokens + tokensToAdd);
        lastRefillTime = now;
    }

public:
    TokenBucket(int capacity, double refillRatePerSecond)
        : capacity(capacity),
          tokens(capacity),
          refillRatePerMs(refillRatePerSecond / 1000.0),
          lastRefillTime(std::chrono::steady_clock::now()) {}

    bool allowRequest() {
        std::lock_guard<std::mutex> lock(mtx);
        refillTokens();
        if (tokens >= 1.0) {
            tokens -= 1.0;
            return true;
        }
        return false;
    }
};


class TokenBucketRateLimiter {
private:
    std::unordered_map<std::string, std::shared_ptr<TokenBucket>> userBuckets;
    std::mutex mapMutex;
    int capacity;
    double refillRatePerSecond;

public:
    TokenBucketRateLimiter(int capacity, double refillRatePerSecond)
        : capacity(capacity), refillRatePerSecond(refillRatePerSecond) {}

    bool allowRequest(const std::string& userId) {
        std::shared_ptr<TokenBucket> bucket;

        // Lock map while accessing/modifying
        {
            std::lock_guard<std::mutex> lock(mapMutex);
            auto it = userBuckets.find(userId);
            if (it == userBuckets.end()) {
                bucket = std::make_shared<TokenBucket>(capacity, refillRatePerSecond);
                userBuckets[userId] = bucket;
            } else {
                bucket = it->second;
            }
        }

        return bucket->allowRequest();
    }
};


void testTokenBucket(TokenBucketRateLimiter& limiter, const string& userId, int threadId) {
    if (limiter.allowRequest(userId)) {
        cout << "[TokenBucket] Thread " << threadId << ": Allowed\n";
    } else {
        cout << "[TokenBucket] Thread " << threadId << ": Denied\n";
    }
}


int main() {
    const std::string userId = "user123";
    const int numThreads = 15;

    // === Token Bucket: 10 capacity, 5 tokens per second
    TokenBucketRateLimiter tokenLimiter(10, 5.0); // 5 tokens per second

    cout << "\n=== Testing Token Bucket Rate Limiter ===\n";
    vector<thread> tokenThreads;
    for (int i = 0; i < numThreads; ++i) {
        tokenThreads.emplace_back(testTokenBucket, ref(tokenLimiter), userId, i);
        this_thread::sleep_for(chrono::milliseconds(100)); // small delay between requests
    }

    for (auto& t : tokenThreads) {
        t.join();
    }
    return 0;
}
