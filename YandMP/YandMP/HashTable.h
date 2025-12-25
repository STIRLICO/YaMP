#pragma once
#include "Token.h"
#include <iostream>

template <typename T>
class HashTable {
private:
    //Вхождение хэш таблицы
    struct Entry {
        int key;
        T value;
        bool occupied;//Занята ли ячейка
        bool deleted;//Очищена ли ячейка

        Entry() : key(0), occupied(false), deleted(false) {}
    };

    Entry* table;
    int capacity;//Действительный размер
    int size;//Размер

    

public:
    //Хэш функция
    int hashFunc(int key) const {
        return (key & 0x7FFFFFFF) % capacity;//Бинарное И Между ключом и 0x7FFFFFFF и остаток от деления получившегося числа
    }
    int hashFunc(const std::string& s) const {
        unsigned long h = 0;
        for (unsigned char c : s) h = h * 131 + c;
        return static_cast<int>(h & 0x7fffffff);
    }

    //Создание хэш таблицы, по умолчанию вместимость 211
    HashTable(int cap = 211) : capacity(cap), size(0) {
        table = new Entry[capacity];
    }

    ~HashTable() { delete[] table; }

    //Вставка нового элемента
    int insert(int key, const T& value) {
        int idx = hashFunc(key);
        int start = idx;

        //Сначала ищем существующую запись с такой же лексемой
        do {
            if (table[idx].occupied and !table[idx].deleted) {
                if (table[idx].value.getLexeme() == value.getLexeme()) {
                    return idx;//Лексема уже существует, возвращаем ее индекс
                }
            }
            idx = (idx + 1) % capacity;//сдвигаем индекс
        } while (idx != start and table[idx].occupied);//Если мы не вернулись в начало и ячейка занята

        //Таблица заполнена и в ней нет места для новой лексемы
        if (table[idx].occupied and !table[idx].deleted) {
            return -1;
        }

        //Вставляем новую запись
        table[idx].key = key;
        table[idx].value = value;
        table[idx].value.setIndex(idx); //Устанавливаем индекс в самом токене
        table[idx].occupied = true;
        table[idx].deleted = false;
        size++;

        return idx;
    }

    //Поиск по индексу
    const Entry* atIndex(int idx) const {
        if (idx < 0 or idx >= capacity) return nullptr;
        if (table[idx].occupied and !table[idx].deleted) return &table[idx];
        return nullptr;
    }

    //Получаем размер
    int getCapacity() const { return capacity; }

    //Вывод хэш таблицы в файл
    void printToStream(std::ostream& os) const {
        for (int i = 0; i < capacity; ++i) {
            if (table[i].occupied && !table[i].deleted) {
                os << table[i].value.typeToString() << " | "
                    << table[i].value.getLexeme() << " | " << i << "\n";
            }
        }
    }
};