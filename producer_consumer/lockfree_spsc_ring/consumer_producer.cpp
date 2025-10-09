#include <iostream>
#include <vector>
#include <atomic>
#include <thread>
#include <chrono>

// The type of data we'll store
using Data = int;

class LockFreeSPSCRingBuffer {
private:
    // The buffer uses one extra element to distinguish between full and empty
    std::vector<Data> buffer_; 
    size_t capacity_;
    
    // The indices must be atomic for thread-safe reading/writing
    // We use std::atomic<size_t> instead of a mutex
    std::atomic<size_t> head_{0}; // Producer's index (where to write next)
    std::atomic<size_t> tail_{0}; // Consumer's index (where to read next)

public:
    LockFreeSPSCRingBuffer(size_t capacity) : capacity_(capacity + 1), buffer_(capacity + 1) {
        if (capacity == 0) {
            // Internal capacity is size + 1
            throw std::invalid_argument("Capacity must be greater than 0");
        }
        std::cout << "Buffer initialized with capacity for " << capacity << " items." << std::endl;
    }

    // Producer method: Attempts to add an element to the buffer
    // Returns true on success, false if the buffer is full
    bool try_push(const Data& item) {
        // 1. Load the current indices
        const size_t current_head = head_.load(std::memory_order_relaxed);
        const size_t next_head = (current_head + 1) % capacity_;

        // 2. Check for FULL condition
        // The buffer is full if advancing head makes it equal to tail
        if (next_head == tail_.load(std::memory_order_acquire)) {
            // Failed: Buffer is full
            return false;
        }

        // 3. Write the data (DATA access)
        // Store the data into the calculated next head position
        buffer_[current_head] = item;

        // 4. Update the head index (INDEX access)
        // We use RELEASE ordering to ensure the data write (step 3) completes
        // BEFORE the head index is made visible to the Consumer thread.
        head_.store(next_head, std::memory_order_release);
        
        return true;
    }

    // Consumer method: Attempts to read an element from the buffer
    // Returns true on success, false if the buffer is empty
    bool try_pop(Data& item) {
        // 1. Load the current indices
        const size_t current_tail = tail_.load(std::memory_order_relaxed);

        // 2. Check for EMPTY condition
        // The buffer is empty if head and tail are equal
        if (current_tail == head_.load(std::memory_order_acquire)) {
            // Failed: Buffer is empty
            return false;
        }

        // 3. Read the data (DATA access)
        // We use ACQUIRE ordering (above, when loading head) to ensure this data read
        // happens AFTER the Producer's data write. This ensures we read valid data.
        item = buffer_[current_tail];

        // 4. Update the tail index (INDEX access)
        const size_t next_tail = (current_tail + 1) % capacity_;
        tail_.store(next_tail, std::memory_order_release);
        
        return true;
    }
};

// --- Example Usage ---

void producer_func(LockFreeSPSCRingBuffer& buffer, int count) {
    std::cout << "Producer started." << std::endl;
    for (int i = 1; i <= count; ++i) {
        // Busy-wait if buffer is full (typical in HFT to avoid sleeping)
        while (!buffer.try_push(i)) {
            std::this_thread::yield(); // Be nice to other threads
        }
        // std::cout << "Produced: " << i << std::endl;
        // Introduce a small delay to allow the consumer to fall behind/catch up
        if (i % 100000 == 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }
    std::cout << "Producer finished producing " << count << " items." << std::endl;
}

void consumer_func(LockFreeSPSCRingBuffer& buffer, int expected_count) {
    std::cout << "Consumer started." << std::endl;
    int received_count = 0;
    Data item;
    
    // Loop until we have received all expected items
    while (received_count < expected_count) {
        if (buffer.try_pop(item)) {
            // std::cout << "Consumed: " << item << std::endl;
            received_count++;
            if (item != received_count) {
                std::cerr << "ERROR: Received value " << item << " but expected " << received_count << std::endl;
            }
        } else {
            std::this_thread::yield(); // Be nice to other threads
        }
    }
    std::cout << "Consumer finished consuming " << received_count << " items." << std::endl;
}

int main() {
    const int ITEM_COUNT = 500000;
    LockFreeSPSCRingBuffer buffer(1024); // Capacity of 1024 items

    // Start the threads
    std::thread producer(producer_func, std::ref(buffer), ITEM_COUNT);
    std::thread consumer(consumer_func, std::ref(buffer), ITEM_COUNT);

    // Wait for them to finish
    producer.join();
    consumer.join();

    std::cout << "\nLock-free SPSC Ring Buffer Test Complete." << std::endl;
    return 0;
}
