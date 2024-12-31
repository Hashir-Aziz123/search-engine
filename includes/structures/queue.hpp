#include <iostream>
#include <stdexcept>

template <typename T>
class Queue {
private:
    T* data;
    size_t capacity;
    size_t frontIndex;
    size_t rearIndex;
    size_t count;

public:
    explicit Queue(size_t cap = 10) 
        : capacity(cap), frontIndex(0), rearIndex(cap - 1), count(0) {
        data = new T[capacity];
    }

    ~Queue() {
        delete[] data;
    }

    void push(const T& value) {
        if (full()) {
            throw std::overflow_error("Queue is full");
        }
        rearIndex = (rearIndex + 1) % capacity;
        data[rearIndex] = value;
        ++count;
    }

    void pop() {
        if (empty()) {
            throw std::underflow_error("Queue is empty");
        }
        frontIndex = (frontIndex + 1) % capacity;
        --count;
    }

    T& front() {
        if (empty()) {
            throw std::underflow_error("Queue is empty");
        }
        return data[frontIndex];
    }

    T& back() {
        if (empty()) {
            throw std::underflow_error("Queue is empty");
        }
        return data[rearIndex];
    }

    bool empty() const {
        return count == 0;
    }

    bool full() const {
        return count == capacity;
    }

    size_t size() const {
        return count;
    }

    void clear() {
        frontIndex = 0;
        rearIndex = capacity - 1;
        count = 0;
    }

    void print() const {
        if (empty()) {
            std::cout << "Queue is empty" << std::endl;
            return;
        }

        size_t index = frontIndex;
        for (size_t i = 0; i < count; ++i) {
            std::cout << data[index] << " ";
            index = (index + 1) % capacity;
        }
        std::cout << std::endl;
    }
};