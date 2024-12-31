#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#include <iostream>
#include <vector>
#include <list>
#include <functional>
#include <utility>
#include <stdexcept>
#include <fstream>
#include "structures/list.hpp"

//initial hashmap size
#define INIT_MAP_SIZE 100

template <typename KeyType, typename ValueType>
class HashMap {
private:
    // A single bucket is a linked list of pairs
    typedef std::pair<KeyType, ValueType> Pair;
    typedef CustomList<Pair> Bucket;

    std::vector<Bucket> buckets;
    size_t bucketCount;
    size_t size;
    const double loadFactorThreshold = 0.75; // Load factor at which the table is rehashed

    size_t getBucketIndex(const KeyType &key) const {
        return std::hash<KeyType>{}(key) % bucketCount;
    }

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
    HashMap(size_t initialBucketCount = INIT_MAP_SIZE) : bucketCount(initialBucketCount), size(0) {
        buckets.resize(bucketCount);
    }

    void insert(const std::pair<KeyType, ValueType> &keyValuePair) {
        if ((double)size / bucketCount >= loadFactorThreshold) {
            rehash();
        }

        size_t index = getBucketIndex(keyValuePair.first);
        for (auto &pair : buckets[index]) {
            if (pair.first == keyValuePair.first) {
                pair.second = keyValuePair.second;
                return;
            }
        }
        buckets[index].emplace_back(keyValuePair);
        ++size;
    }

    ValueType* find(const KeyType &key) {
        size_t index = getBucketIndex(key);
        for (auto &pair : buckets[index]) {
            if (pair.first == key) {
                return &pair.second;
            }
        }
        return nullptr; // Key not found
    }


    // ValueType *find(const KeyType &key) const {
    //     size_t index = getBucketIndex(key);
    //     for (auto &pair : buckets[index]) {
    //         if (pair.first == key) {
    //             return &pair.second;
    //         }
    //     }
    //     return nullptr; // Key not found
    // }

    ValueType &at(const KeyType &key) const {
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
        return false;
    }

    ValueType &operator[](const KeyType &key) {
        size_t index = getBucketIndex(key);
        for (auto &pair : buckets[index]) {
            if (pair.first == key) {
                return pair.second;
            }
        }

        buckets[index].emplace_back(key, ValueType());
        ++size;
        return buckets[index].back().second;
    }

    size_t getSize() const {
        return size;
    }

    bool empty() const {
        return size == 0;
    }

    std::vector<Bucket> &getTable() {
        return buckets;
    }

    //Iterator
    class Iterator {
    private:
        const std::vector<Bucket> *buckets;
        size_t bucketIndex;
        typename Bucket::const_iterator listIterator;

        void advanceToNext() {
            while (bucketIndex < buckets->size() && listIterator == (*buckets)[bucketIndex].end()) {
                ++bucketIndex;
                if (bucketIndex < buckets->size()) {
                    listIterator = (*buckets)[bucketIndex].begin();
                }
            }
        }

    public:
        Iterator(const std::vector<Bucket> *buckets, size_t bucketIndex, typename Bucket::const_iterator listIterator)
            : buckets(buckets), bucketIndex(bucketIndex), listIterator(listIterator) {
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

    Iterator begin() const {
        return Iterator(&buckets, 0, buckets[0].begin());
    }

    Iterator end() const {
        return Iterator(&buckets, buckets.size(), buckets.back().end());
    }

    Iterator findIterator(const KeyType &key) {
        size_t index = getBucketIndex(key);
        for (auto it = buckets[index].begin(); it != buckets[index].end(); ++it) {
            if (it->first == key) {
                return Iterator(&buckets, index, it);
            }
        }
        return end();
    }
};

#endif