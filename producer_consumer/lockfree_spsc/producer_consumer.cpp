#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <mutex>
#include <random> // Modern, thread-safe random number generation
#include <ctime>  // For std::time

// Mutex remains for protecting the shared console output (std::cout)
std::mutex cout_mu;

// --- LockFreeSPSCQueue Class (Minor fix in remove) ---

class LockFreeSPSCQueue
{
private:
    std::vector<int> buffer_;
    const size_t size_; // Max capacity + 1 (to distinguish empty/full)
    
    // head_ is only written by the consumer and read by the producer/consumer.
    std::atomic<size_t> head_ {0}; 
    
    // tail_ is only written by the producer and read by the producer/consumer.
    std::atomic<size_t> tail_ {0}; 

public:
    LockFreeSPSCQueue(unsigned int capacity) 
    : size_(capacity + 1), buffer_(capacity + 1) {}

    void add(int num) {
        size_t current_tail = tail_.load(std::memory_order_relaxed);
        size_t next_tail = (current_tail + 1) % size_;
        
        // Busy wait until there is space
        while (next_tail == head_.load(std::memory_order_acquire)) {
            std::this_thread::yield(); 
        }

        // Write the data
        buffer_[current_tail] = num;
        
        // Move the tail index (Release ensures data is visible)
        tail_.store(next_tail, std::memory_order_release);
    }
    
    int remove() {
        int result;
        size_t current_head;

        while (true) {
            // Read the head index (relaxed for initial check)
            current_head = head_.load(std::memory_order_relaxed);
            
            // Check if queue is empty (head == tail)
            // Acquire ensures the read sees the latest producer write
            if (current_head == tail_.load(std::memory_order_acquire)) {
                // Queue is empty, busy wait for the producer
                std::this_thread::yield();
            } else {
                // Data is available, break the wait loop
                break;
            }
        }

        // Read the data
        result = buffer_[current_head];
        
        // FIX: The index update logic should use current_head, not next_head 
        // to calculate the *next* head value. The old code had next_head calculation 
        // inside the loop, which wasn't necessary.
        size_t next_head = (current_head + 1) % size_;
        
        // Move the head index (Release ensures the space is visible to the producer)
        head_.store(next_head, std::memory_order_release);
        return result;
    }
};

// --- Producer Class (Fixed for thread-safe random number generation) ---

class Producer
{
private:
    LockFreeSPSCQueue *buffer_;
    std::string name_;
    // Thread-safe random engine and distribution
    std::mt19937 rand_engine;
    std::uniform_int_distribution<int> num_dist{0, 99}; // 0 to 99 for produced number
    std::uniform_int_distribution<int> sleep_dist{0, 99}; // 0 to 99 for sleep time

public:
    Producer(LockFreeSPSCQueue* buffer, std::string name)
    : buffer_(buffer), name_(name)
    {
        // Seed the engine uniquely using high-resolution clock
        rand_engine.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    }
    
    void run() {
        while (true) {
            // Generate numbers using the thread's private engine
            int num = num_dist(rand_engine);
            buffer_->add(num);
            
            // Output protection remains a necessity
            std::lock_guard<std::mutex> lock(cout_mu);
            int sleep_time = sleep_dist(rand_engine);
            std::cout << "Name: " << name_ << "    Produced: " << num << "    Sleep time: " << sleep_time << "ms" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
        }
    }
};

// --- Consumer Class (Fixed for thread-safe random number generation) ---

class Consumer
{
private:
    LockFreeSPSCQueue *buffer_;
    std::string name_;
    // Thread-safe random engine and distribution
    std::mt19937 rand_engine;
    std::uniform_int_distribution<int> sleep_dist{0, 99}; // 0 to 99 for sleep time
    
public:
    Consumer(LockFreeSPSCQueue* buffer, std::string name)
    : buffer_(buffer), name_(name)
    {
        // Seed the engine uniquely using high-resolution clock
        rand_engine.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count() + 1); // +1 ensures different seed
    }
    
    void run() {
        while (true) {
            int num = buffer_->remove();
            
            // Output protection remains a necessity
            std::lock_guard<std::mutex> lock(cout_mu);
            int sleep_time = sleep_dist(rand_engine);
            std::cout << "Name: " << name_ << "    Consumed: " << num << "    Sleep time: " << sleep_time << "ms" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
        }
    }
};

// --- Main Function ---

int main() {
    // We only need ctime for the random seed, but high_resolution_clock is better.
    // The srand() in main is also unnecessary now.
    
    LockFreeSPSCQueue b(10);
    Producer p1(&b, "Producer_SPSC");
    Consumer c1(&b, "Consumer_SPSC");

    std::thread producer_thread(&Producer::run, &p1);
    std::thread consumer_thread(&Consumer::run, &c1);
    
    producer_thread.join();
    consumer_thread.join();

    return 0;
}
