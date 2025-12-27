#pragma once
#include <iostream>

template <typename T>
class HashTable {
private:
    struct Entry {
        int key;
        T value;
        bool occupied;
        bool deleted;

        Entry() : key(0), occupied(false), deleted(false) {}
    };

    Entry* table;
    int capacity;
    int size;

public:
    int hashFunc(int key) const {
        return (key & 0x7FFFFFFF) % capacity;
    }

    int hashFunc(const std::string& s) const {
        unsigned long h = 0;
        for (unsigned char c : s) h = h * 131 + c;
        return static_cast<int>(h & 0x7fffffff);
    }

    HashTable(int cap = 211) : capacity(cap), size(0) {
        table = new Entry[capacity];
    }

    ~HashTable() { delete[] table; }

    int insert(const std::string& keyStr, const T& value) {
        int key = hashFunc(keyStr);
        int idx = hashFunc(key);
        int start = idx;

        do {
            if (table[idx].occupied && !table[idx].deleted) {
                if (table[idx].value.getKey() == keyStr) {
                    return idx;
                }
            }
            idx = (idx + 1) % capacity;
        } while (idx != start && table[idx].occupied);

        if (table[idx].occupied && !table[idx].deleted) {
            return -1;
        }

        table[idx].key = key;
        table[idx].value = value;
        table[idx].occupied = true;
        table[idx].deleted = false;
        size++;

        return idx;
    }

    const Entry* atIndex(int idx) const {
        if (idx < 0 || idx >= capacity) return nullptr;
        if (table[idx].occupied && !table[idx].deleted) return &table[idx];
        return nullptr;
    }

    int findIndex(const std::string& keyStr) const {
        int key = hashFunc(keyStr);
        int idx = hashFunc(key);
        int start = idx;

        do {
            if (table[idx].occupied && !table[idx].deleted) {
                if (table[idx].value.getKey() == keyStr) {
                    return idx;
                }
            }
            idx = (idx + 1) % capacity;
        } while (idx != start && table[idx].occupied);

        return -1;
    }

    T* getValue(int idx) {
        if (idx < 0 || idx >= capacity) return nullptr;
        if (table[idx].occupied && !table[idx].deleted) return &table[idx].value;
        return nullptr;
    }

    const T* getValue(int idx) const {
        if (idx < 0 || idx >= capacity) return nullptr;
        if (table[idx].occupied && !table[idx].deleted) return &table[idx].value;
        return nullptr;
    }

    int getCapacity() const { return capacity; }
    int getSize() const { return size; }

    void printToStream(std::ostream& os) const {
        for (int i = 0; i < capacity; ++i) {
            if (table[i].occupied && !table[i].deleted) {
                os << table[i].value.typeToString() << " | "
                    << table[i].value.getKey() << " | " << i << "\n";
            }
        }
    }
};