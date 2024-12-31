#ifndef _CUSTOM_LINKED_LIST_H_
#define _CUSTOM_LINKED_LIST_H_

#include <iterator>
#include <utility>
#include <cstddef>

template <typename T>
class CustomList {
private:
    struct Node {
        T data;
        Node *prev;
        Node *next;

        template <typename... Args>
        Node(Args &&...args) : data(std::forward<Args>(args)...), prev(nullptr), next(nullptr) {}
    };

    Node *head;
    Node *tail;
    size_t listSize;

public:
    CustomList() : head(nullptr), tail(nullptr), listSize(0) {}

    ~CustomList() { clear(); }

    CustomList(const CustomList &) = delete;
    CustomList &operator=(const CustomList &) = delete;

    CustomList(CustomList &&other) noexcept : head(other.head), tail(other.tail), listSize(other.listSize) {
        other.head = nullptr;
        other.tail = nullptr;
        other.listSize = 0;
    }

    CustomList &operator=(CustomList &&other) noexcept {
        if (this != &other) {
            clear();
            head = other.head;
            tail = other.tail;
            listSize = other.listSize;
            other.head = nullptr;
            other.tail = nullptr;
            other.listSize = 0;
        }
        return *this;
    }

    class Iterator {
    private:
        Node *current;

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T *;
        using reference = T &;

        explicit Iterator(Node *node) : current(node) {}

        T &operator*() { return current->data; }
        T *operator->() { return &current->data; }

        Iterator &operator++() {
            current = current->next;
            return *this;
        }

        Iterator operator++(int) {
            Iterator temp = *this;
            ++(*this);
            return temp;
        }

        Iterator &operator--() {
            current = current->prev;
            return *this;
        }

        Iterator operator--(int) {
            Iterator temp = *this;
            --(*this);
            return temp;
        }

        bool operator==(const Iterator &other) const { return current == other.current; }
        bool operator!=(const Iterator &other) const { return current != other.current; }
    };

    using iterator = Iterator;

    class ConstIterator {
    private:
        const Node *current;

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T *;
        using reference = const T &;

        explicit ConstIterator(const Node *node) : current(node) {}

        const T &operator*() const { return current->data; }
        const T *operator->() const { return &current->data; }

        ConstIterator &operator++() {
            current = current->next;
            return *this;
        }

        ConstIterator operator++(int) {
            ConstIterator temp = *this;
            ++(*this);
            return temp;
        }

        ConstIterator &operator--() {
            current = current->prev;
            return *this;
        }

        ConstIterator operator--(int) {
            ConstIterator temp = *this;
            --(*this);
            return temp;
        }

        bool operator==(const ConstIterator &other) const { return current == other.current; }
        bool operator!=(const ConstIterator &other) const { return current != other.current; }
    };

    using const_iterator = ConstIterator;

    // Begin iterator
    iterator begin() { return iterator(head); }
    iterator end() { return iterator(nullptr); }

    const_iterator begin() const { return const_iterator(head); }
    const_iterator end() const { return const_iterator(nullptr); }

    void push_back(const T &value) {
        Node *newNode = new Node(value);
        if (!head) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
        ++listSize;
    }

    template <typename... Args>
    void emplace_back(Args &&...args) {
        Node *newNode = new Node(std::forward<Args>(args)...);
        if (!head) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
        ++listSize;
    }

    iterator erase(iterator it) {
        Node *nodeToDelete = it.current;
        if (!nodeToDelete) return end();

        if (nodeToDelete->prev) {
            nodeToDelete->prev->next = nodeToDelete->next;
        } else {
            head = nodeToDelete->next;
        }

        if (nodeToDelete->next) {
            nodeToDelete->next->prev = nodeToDelete->prev;
        } else {
            tail = nodeToDelete->prev;
        }

        iterator nextIt(nodeToDelete->next);
        delete nodeToDelete;
        --listSize;
        return nextIt;
    }

    void clear() {
        Node *current = head;
        while (current) {
            Node *next = current->next;
            delete current;
            current = next;
        }
        head = tail = nullptr;
        listSize = 0;
    }

    T &back() {
        if (!tail) {
            throw std::out_of_range("List is empty");
        }
        return tail->data;
    }

    const T &back() const {
        if (!tail) {
            throw std::out_of_range("List is empty");
        }
        return tail->data;
    }

    size_t size() const { return listSize; }

    bool empty() const { return listSize == 0; }
};

#endif

// #ifndef _CUSTOM_LINKED_LIST_H_
// #define _CUSTOM_LINKED_LIST_H_

// #include <iterator>
// #include <utility>
// #include <cstddef>

// // A custom doubly-linked list implementation
// template <typename T>
// class CustomList {
// private:
//     // Node definition
//     struct Node {
//         T data;
//         Node *prev;
//         Node *next;

//         // Variadic template constructor for in-place construction
//         template <typename... Args>
//         Node(Args &&...args) : data(std::forward<Args>(args)...), prev(nullptr), next(nullptr) {}
//     };

//     Node *head;
//     Node *tail;
//     size_t listSize;

// public:
//     // Constructor
//     CustomList() : head(nullptr), tail(nullptr), listSize(0) {}

//     // Destructor
//     ~CustomList() { clear(); }

//     // Disable copy and assignment
//     CustomList(const CustomList &) = delete;
//     CustomList &operator=(const CustomList &) = delete;

//     // Move constructor and assignment
//     CustomList(CustomList &&other) noexcept : head(other.head), tail(other.tail), listSize(other.listSize) {
//         other.head = nullptr;
//         other.tail = nullptr;
//         other.listSize = 0;
//     }

//     CustomList &operator=(CustomList &&other) noexcept {
//         if (this != &other) {
//             clear();
//             head = other.head;
//             tail = other.tail;
//             listSize = other.listSize;
//             other.head = nullptr;
//             other.tail = nullptr;
//             other.listSize = 0;
//         }
//         return *this;
//     }

//     // Iterator definition
//     class Iterator {
//     private:
//         Node *current;

//     public:
//         using iterator_category = std::bidirectional_iterator_tag;
//         using value_type = T;
//         using difference_type = std::ptrdiff_t;
//         using pointer = T *;
//         using reference = T &;

//         explicit Iterator(Node *node) : current(node) {}

//         T &operator*() { return current->data; }
//         T *operator->() { return &current->data; }

//         Iterator &operator++() {
//             current = current->next;
//             return *this;
//         }

//         Iterator operator++(int) {
//             Iterator temp = *this;
//             ++(*this);
//             return temp;
//         }

//         Iterator &operator--() {
//             current = current->prev;
//             return *this;
//         }

//         Iterator operator--(int) {
//             Iterator temp = *this;
//             --(*this);
//             return temp;
//         }

//         bool operator==(const Iterator &other) const { return current == other.current; }
//         bool operator!=(const Iterator &other) const { return current != other.current; }
//     };

//     using iterator = Iterator;

//     // Const iterator definition
//     class ConstIterator {
//     private:
//         const Node *current;

//     public:
//         using iterator_category = std::bidirectional_iterator_tag;
//         using value_type = T;
//         using difference_type = std::ptrdiff_t;
//         using pointer = const T *;
//         using reference = const T &;

//         explicit ConstIterator(const Node *node) : current(node) {}

//         const T &operator*() const { return current->data; }
//         const T *operator->() const { return &current->data; }

//         ConstIterator &operator++() {
//             current = current->next;
//             return *this;
//         }

//         ConstIterator operator++(int) {
//             ConstIterator temp = *this;
//             ++(*this);
//             return temp;
//         }

//         ConstIterator &operator--() {
//             current = current->prev;
//             return *this;
//         }

//         ConstIterator operator--(int) {
//             ConstIterator temp = *this;
//             --(*this);
//             return temp;
//         }

//         bool operator==(const ConstIterator &other) const { return current == other.current; }
//         bool operator!=(const ConstIterator &other) const { return current != other.current; }
//     };

//     using const_iterator = ConstIterator;

//     // Begin iterator
//     iterator begin() { return iterator(head); }
//     iterator end() { return iterator(nullptr); }

//     const_iterator begin() const { return const_iterator(head); }
//     const_iterator end() const { return const_iterator(nullptr); }

//     // Push back
//     void push_back(const T &value) {
//         Node *newNode = new Node(value);
//         if (!head) {
//             head = tail = newNode;
//         } else {
//             tail->next = newNode;
//             newNode->prev = tail;
//             tail = newNode;
//         }
//         ++listSize;
//     }

//     // Emplace back
//     template <typename... Args>
//     void emplace_back(Args &&...args) {
//         Node *newNode = new Node(std::forward<Args>(args)...);
//         if (!head) {
//             head = tail = newNode;
//         } else {
//             tail->next = newNode;
//             newNode->prev = tail;
//             tail = newNode;
//         }
//         ++listSize;
//     }

//     // Erase a node by iterator
//     iterator erase(iterator it) {
//         Node *nodeToDelete = it.current;
//         if (!nodeToDelete) return end();

//         if (nodeToDelete->prev) {
//             nodeToDelete->prev->next = nodeToDelete->next;
//         } else {
//             head = nodeToDelete->next;
//         }

//         if (nodeToDelete->next) {
//             nodeToDelete->next->prev = nodeToDelete->prev;
//         } else {
//             tail = nodeToDelete->prev;
//         }

//         iterator nextIt(nodeToDelete->next);
//         delete nodeToDelete;
//         --listSize;
//         return nextIt;
//     }

//     // Clear the list
//     void clear() {
//         Node *current = head;
//         while (current) {
//             Node *next = current->next;
//             delete current;
//             current = next;
//         }
//         head = tail = nullptr;
//         listSize = 0;
//     }

//     // Size
//     size_t size() const { return listSize; }

//     // Check if empty
//     bool empty() const { return listSize == 0; }
// };

// #endif
