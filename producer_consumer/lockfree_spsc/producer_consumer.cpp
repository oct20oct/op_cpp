#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <cstdlib> // For std::rand and std::srand

// Mutex remains for protecting the shared console output (std::cout)
std::mutex cout_mu;

/**
 * @class LockFreeSPSCQueue
 * @brief Implements a thread-safe, lock-free, Single-Producer Single-Consumer (SPSC)
 * bounded queue using atomic indices and busy-waiting/yielding.
 * * NOTE: This is NOT safe for multiple producers or multiple consumers.
 * The busy-wait loop using std::this_thread::yield() replaces the efficient
 * blocking mechanism of std::condition_variable.
 */
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
    // Initialize the buffer with capacity + 1 slots.
    LockFreeSPSCQueue(unsigned int capacity) 
    : size_(capacity + 1), buffer_(capacity + 1) {}

    /**
     * @brief Adds an item to the buffer (Producer operation).
     * The producer busy-waits until space is available.
     * @param num The item to add.
     */
    void add(int num) {
        // Read the tail index (producer's exclusive write index)
        size_t current_tail = tail_.load(std::memory_order_relaxed);
        size_t next_tail = (current_tail + 1) % size_;
        
        // Busy wait until there is space (next_tail must not equal head_)
        while (next_tail == head_.load(std::memory_order_acquire)) {
            // Yield the thread's execution time slice to allow the consumer to run.
            std::this_thread::yield(); 
        }

        // Write the data
        buffer_[current_tail] = num;
        
        // Move the tail index. memory_order_release ensures the data write (above)
        // is visible to the consumer before the index update.
        tail_.store(next_tail, std::memory_order_release);
    }
    
    /**
     * @brief Removes and returns an item from the buffer (Consumer operation).
     * The consumer busy-waits until an item is available.
     * @return The item removed from the buffer.
     */
    int remove() {
        int result;
        size_t current_head;
        size_t next_head;

        while (true) {
            // Read the head index (consumer's exclusive write index)
            current_head = head_.load(std::memory_order_relaxed);
            
            // Check if queue is empty (head == tail)
            // memory_order_acquire ensures the read sees the latest write from the producer.
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
        next_head = (current_head + 1) % size_;
        
        // Move the head index. memory_order_release ensures the data read (above)
        // is visible to the producer before the index update.
        head_.store(next_head, std::memory_order_release);
        return result;
    }
};


class Producer
{
public:
    Producer(LockFreeSPSCQueue* buffer, std::string name)
    {
        this->buffer_ = buffer;
        this->name_ = name;
        // Seed the random number generator
        std::srand(std::time(nullptr) + (size_t)this);
    }
    void run() {
        while (true) {
            int num = std::rand() % 100;
            buffer_->add(num);
            
            // Output protection remains a necessity
            std::lock_guard<std::mutex> lock(cout_mu);
            int sleep_time = std::rand() % 100;
            std::cout << "Name: " << name_ << "    Produced: " << num << "    Sleep time: " << sleep_time << "ms" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
        }
    }
private:
    LockFreeSPSCQueue *buffer_;
    std::string name_;
};

class Consumer
{
public:
    Consumer(LockFreeSPSCQueue* buffer, std::string name)
    {
        this->buffer_ = buffer;
        this->name_ = name;
        std::srand(std::time(nullptr) + (size_t)this + 1);
    }
    void run() {
        while (true) {
            int num = buffer_->remove();
            
            // Output protection remains a necessity
            std::lock_guard<std::mutex> lock(cout_mu);
            int sleep_time = std::rand() % 100;
            std::cout << "Name: " << name_ << "    Consumed: " << num << "    Sleep time: " << sleep_time << "ms" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
        }
    }
private:
    LockFreeSPSCQueue *buffer_;
    std::string name_;
};

int main() {
    // Set up one producer and one consumer for the SPSC queue
    LockFreeSPSCQueue b(10);
    Producer p1(&b, "Producer_SPSC");
    Consumer c1(&b, "Consumer_SPSC");

    std::thread producer_thread(&Producer::run, &p1);
    std::thread consumer_thread(&Consumer::run, &c1);
    
    // We intentionally run forever in this producer/consumer loop, so join is used 
    // to block main and keep the threads alive.
    producer_thread.join();
    consumer_thread.join();

    return 0;
}
