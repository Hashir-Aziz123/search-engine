#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#include <iostream>
#include <vector>
#include <list>
#include <functional>
#include <utility>
#include <stdexcept>
#include <fstream>

//initial hashmap size
#define INIT_MAP_SIZE 100

template <typename KeyType, typename ValueType>
class HashMap {
private:
    // Define a bucket as a list of pairs (key, value)
    typedef std::pair<KeyType, ValueType> Pair;
    typedef std::list<Pair> Bucket;

    std::vector<Bucket> buckets; // Vector of buckets
    size_t bucketCount;          // Number of buckets
    size_t size;                 // Total number of elements
    const double loadFactorThreshold = 0.75; // Threshold to trigger rehashing

    // Hash function to map keys to indices
    size_t getBucketIndex(const KeyType &key) const {
        return std::hash<KeyType>{}(key) % bucketCount;
    }

    // Rehashing: Expand and reinsert all elements into new buckets
    void rehash() {
        size_t newBucketCount = bucketCount * 2;
        std::vector<Bucket> newBuckets(newBucketCount);

        for (const auto &bucket : buckets) {
            for (const auto &pair : bucket) {
                size_t newIndex = std::hash<KeyType>{}(pair.first) % newBucketCount;
                newBuckets[newIndex].push_back(pair);
            }
        }

        buckets = std::move(newBuckets);
        bucketCount = newBucketCount;
    }

public:
    // Constructor
    HashMap(size_t initialBucketCount = INIT_MAP_SIZE) : bucketCount(initialBucketCount), size(0) {
        buckets.resize(bucketCount);
    }

    // Insert a key-value pair
    void insert(const std::pair<KeyType, ValueType> &keyValuePair) {
        if ((double)size / bucketCount >= loadFactorThreshold) {
            rehash(); // Rehash when load factor exceeds threshold
        }

        size_t index = getBucketIndex(keyValuePair.first);
        for (auto &pair : buckets[index]) {
            if (pair.first == keyValuePair.first) {
                pair.second = keyValuePair.second; // Update existing key
                return;
            }
        }
        buckets[index].emplace_back(keyValuePair); // Insert new key-value pair
        ++size;
    }

    // Find value by key
    ValueType *find(const KeyType &key) {
        size_t index = getBucketIndex(key);
        for (auto &pair : buckets[index]) {
            if (pair.first == key) {
                return &pair.second; // Return pointer to value
            }
        }
        return nullptr; // Key not found
    }

    ValueType &at(const KeyType &key) {
        return *find(key);
    }

    // Returns number of elements matching a specific key
    size_t count(const KeyType &key) {
        size_t count = 0;
        for (const auto &bucket : buckets) {
            for (const auto &pair : bucket) {
                if (key == pair.first) count++;
            }
        }

        return count;
    }

    // Erase a key-value pair
    bool erase(const KeyType &key) {
        size_t index = getBucketIndex(key);
        auto &bucket = buckets[index];
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->first == key) {
                bucket.erase(it);
                --size;
                return true;
            }
        }
        return false; // Key not found
    }

    // Operator[] for easy access (like std::unordered_map)
    ValueType &operator[](const KeyType &key) {
        size_t index = getBucketIndex(key);
        for (auto &pair : buckets[index]) {
            if (pair.first == key) {
                return pair.second; // Return existing value
            }
        }

        // If key does not exist, insert it with a default value
        buckets[index].emplace_back(key, ValueType());
        ++size;
        return buckets[index].back().second;
    }

    // Get current size
    size_t getSize() const {
        return size;
    }

    // Check if the hashmap is empty
    bool empty() const {
        return size == 0;
    }

    std::vector<Bucket> &getTable() {
        return buckets;
    }

    //Iterator
        // --- Iterator Implementation ---
    class Iterator {
    private:
        const std::vector<Bucket> *buckets;
        size_t bucketIndex;
        typename Bucket::const_iterator listIterator;

        // Advance the iterator to the next valid position
        void advanceToNext() {
            while (bucketIndex < buckets->size() && listIterator == (*buckets)[bucketIndex].end()) {
                ++bucketIndex;
                if (bucketIndex < buckets->size()) {
                    listIterator = (*buckets)[bucketIndex].begin();
                }
            }
        }

    public:
        // Constructor
        Iterator(const std::vector<Bucket> *buckets, size_t bucketIndex, typename Bucket::const_iterator listIterator)
            : buckets(buckets), bucketIndex(bucketIndex), listIterator(listIterator) {
            // Ensure the iterator is positioned at the first valid element
            advanceToNext();
        }

        const Pair &operator*() const { return *listIterator; }
        const Pair *operator->() const { return &(*listIterator); }

        Iterator &operator++() {
            ++listIterator;
            advanceToNext();
            return *this;
        }

        bool operator==(const Iterator &other) const {
            return bucketIndex == other.bucketIndex && listIterator == other.listIterator;
        }

        bool operator!=(const Iterator &other) const {
            return !(*this == other);
        }
    };

    // Begin iterator
    Iterator begin() const {
        return Iterator(&buckets, 0, buckets[0].begin());
    }

    // End iterator
    Iterator end() const {
        return Iterator(&buckets, buckets.size(), buckets.back().end());
    }
};

#endif
