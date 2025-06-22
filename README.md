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

# Some Implementation details

## why use std::shared_ptr here?

### 1. Shared ownership

Multiple parts of our code may need access to the same TokenBucket object — especially across threads.

Using std::shared_ptr ensures that:

- The TokenBucket stays alive as long as at least one thread or function still needs it.

- The memory is automatically freed when no one is using it anymore.

### 2. Avoid object slicing / duplication

If we stored TokenBucket by value, like this:

std::unordered_map<std::string, TokenBucket> userBuckets;

- Every time we insert or access a bucket, we'd be copying it.

- That creates duplicate state — very bad in a rate limiter, where shared state is essential.

- shared_ptr avoids that — we store and share a single instance per user.

### 3. Thread safety with individual buckets

Each TokenBucket object has its own std::mutex, and allowRequest() uses it to protect internal state.

By giving each thread access to the same TokenBucket instance via shared_ptr, we:

- Keep per-user rate limiting accurate.

- Allow safe, concurrent access to each user's bucket.

### 4. Efficient memory management

No need to manually new or delete anything. shared_ptr takes care of:

- Allocating when we make_shared

- Freeing memory when the last reference goes out of scope

## Uses of emplace_back() instead of push_back()

### push_back()

##### std::vector<std::thread> threads;

##### std::thread t1(myFunc); // construct thread first

##### threads.push_back(t1); // copy or move t1 into the vector

We create the object outside the vector (t1), and then push it in.
This might require copying or moving the object.
With std::thread, which is non-copyable, only move works — still an extra step.

### emplace_back()

##### std::vector<std::thread> threads;

##### threads.emplace*back(myFunc); // construct the thread \_directly inside the vector*

We skip creating the thread outside.
The std::thread is constructed directly in-place, in the vector's memory.
This avoids the extra move step, making it slightly more efficient.
