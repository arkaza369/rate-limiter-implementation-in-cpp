# Sliding Window Log Rate Limiter

The **Sliding Window Log** algorithm is a rate limiting technique that tracks the timestamps of recent requests to enforce limits on how many requests can be made within a rolling time window.

---

## How Sliding Window Log Works

- Each request’s timestamp is recorded in a log (usually a deque or list).
- Before allowing a new request, timestamps older than the sliding time window are removed.
- The number of timestamps within the window is compared against a limit.
- If the count is below the limit, the request is allowed and its timestamp is recorded.
- If the limit is reached or exceeded, the request is denied.

This approach provides an accurate, fine-grained enforcement of rate limits over a moving time window.

---

## Important Details

- The timestamps in the log can represent either:
  - Only **allowed** requests (more memory-efficient), or
  - **All** requests (allowed and denied) to detect abuse or retry patterns.
- Cleaning up old timestamps is essential to keep memory usage bounded.
- Thread safety is required if used in concurrent environments.

---

## Example Code (C++)

```cpp
#include <deque>
#include <chrono>
#include <mutex>
#include <algorithm>

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
```
