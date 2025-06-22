# Token Bucket Rate Limiter

The **Token Bucket** algorithm is a popular rate limiting strategy that controls how many requests a user can make over time.

---

## How it works

- The bucket holds a maximum number of tokens (capacity).
- Tokens are added to the bucket continuously at a fixed rate (refill rate).
- Each request consumes one token.
- If no tokens are available, the request is denied.

This allows bursts of traffic up to the capacity while enforcing a steady average rate.

---

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
