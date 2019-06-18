#ifndef C_PLUS_PLUS_VECTOR_H
#define C_PLUS_PLUS_VECTOR_H

//
// Created by Anarsiel on 2019-06-15.
//

#include <cstddef>
#include <memory>
#include <string.h>

template <typename T>
struct vector {
//    typedef T value_type;
//    typedef ? iterator;
//    typedef ? const_iterator;
//    typedef ? reverse_iterator;
//    typedef ? const_reverse_iterator;

private:
    struct many_elements_type {
        many_elements_type() : capacity(0), links_count(0), array(nullptr) {}

        many_elements_type(T *array_) {
            capacity = 0;
            links_count = 0;
            array = array_;
        }

        many_elements_type(many_elements_type &other)
                : capacity(other.capacity), array(other.array), links_count(other.links_count) {}

        many_elements_type(size_t capacity, size_t links_count, T *array)
                : capacity(capacity), links_count(links_count), array(array) {}

        ~many_elements_type() = default;

        size_t capacity;
        size_t links_count;

        T* array;
    };

public:
    vector() noexcept : sz(0) {};

    vector(T const & element) {
        sz = 1;
        new (&buffer.one_element) T(element);
    }

    vector(vector const &other) {
        sz = other.sz;
        if (other.contains_one()) {
            buffer.one_element = other.buffer.one_element;
        } else if (other.contains_many()) {
            buffer.many_elements = other.buffer.many_elements;
            ++buffer.many_elements->links_count;
        }
    }

    ~vector() {
        if (contains_one()) {
            buffer.one_element.~T();
        } else if (contains_many()) {
            --buffer.many_elements->links_count;
            if (get_amount_of_links() == 0) {
                clear();
                delete (buffer.many_elements);
            }
        }
    }

    template <typename InputIterator>
    vector(InputIterator first, InputIterator last) {
        // todo
    }

    ///////////////////////////////////
    ///////////////////////////////////
    ///////////////////////////////////
    T &operator[](size_t index) {
        if (contains_one()) return buffer.one_element;

        return buffer.many_elements->array[index];
    }

    T const &operator[](size_t index) const {
        if (contains_one()) return buffer.one_element;

        return buffer.many_elements->array[index];
    }

    T &front() {
        if (contains_one()) {
            return buffer.one_element;
        }

        return buffer.many_elements->array[0];
    }

    T &back() {
        if (contains_one()) {
            return buffer.one_element;
        }

        return buffer.many_elements->array[sz - 1];
    }

    void push_back(T const &element) {
        if (empty()) {
            sz = 1;
            new (&buffer.one_element) T(element);
        } else if (contains_one()) {
            make_unique();

            T *tmp_elem = static_cast<T *>(::operator new(sizeof(T)));
            new (tmp_elem) T(buffer.one_element);

            buffer.one_element.~T();

            buffer.many_elements = new many_elements_type(10, 1, static_cast<T *>(::operator new(10 * sizeof(T))));
            sz = 2;

            new (&buffer.many_elements->array[0]) T(*tmp_elem);
//            delete(tmp_elem);

            new (&buffer.many_elements->array[1]) T(element);
        } else if (contains_many()) {
            make_unique();
            ensure_capacity();

            new(buffer.many_elements->array + sz) T(element);
            ++sz;
        }
    }

    void pop_back() {
        if (contains_one()) {
            buffer.one_element.~T();
            --sz;
            return;
        }

        if (sz == 2) {
            make_unique();

            T* tmp = static_cast<T*> (::operator new(sizeof(T)));
            new (tmp) T(buffer.many_elements->array[0]); // this fucking smthing is not initilized!!!!!

            buffer.many_elements->array[0].~T();
            buffer.many_elements->array[1].~T();
            delete (buffer.many_elements);

            new (&buffer.one_element) T(*tmp);
            --sz;
            return;
        }
        make_unique();
        ensure_capacity();

        (buffer.many_elements->array[sz - 1]).~T();
        --sz;
    }

    T* data() {
        if (contains_one()) {
            return &buffer.one_element;
        }
        return buffer.many_elements->array;
    }
    ///////////////////////////////////
    ///////////////////////////////////
    ///////////////////////////////////

    bool empty() const { return !sz; }

    size_t size() const { return sz; }

    size_t capacity() const {
        if (sz == 0 || sz == 1) {
            return sz;
        }

        return get_capacity();
    }

    void clear() {
        while (sz > 0) {
            pop_back();
        }
    }

    vector& operator=(vector const &other) {
        clear();

        sz = other.sz;
        if (other.contains_one()) {
            buffer.one_element = other.buffer.one_element;
        } else if (other.contains_many()) {
            buffer.many_elements = other.buffer.many_elements;
            ++buffer.many_elements->links_count;
        }

        return *this;
    }

private:
    size_t sz;

    union U {
        U() : many_elements(new many_elements_type()) {}
        ~U() {}

        many_elements_type* many_elements;
        T one_element;
    } buffer;

    bool contains_one() const { return sz == 1; }
    bool contains_many() const { return sz > 1; }
    size_t get_capacity() { return buffer.many_elements->capacity; }
    size_t get_amount_of_links() { return buffer.many_elements->links_count; }
    bool unique() { return empty() || contains_one() || (get_amount_of_links() == 1); }

    void make_unique() {
        if (unique()) return;

        auto tmp = new many_elements_type(static_cast<T *>(::operator new(get_capacity() * sizeof(T))));
        tmp->capacity = get_capacity();
        tmp->links_count = 1;

        memcpy(tmp->array, buffer.many_elements->array, get_capacity());

        buffer.many_elements->links_count--;

        buffer.many_elements = tmp;
    }

    // object must be already unique
    void ensure_capacity() {
        if (sz == get_capacity()) {
            T* tmp_adress = static_cast<T *>(::operator new(2 * get_capacity() * sizeof(T)));
            memcpy(tmp_adress, buffer.many_elements->array, sz * sizeof(T));

            buffer.many_elements->array = tmp_adress;
            buffer.many_elements->capacity *= 2;
        }
    }
};

#endif //C_PLUS_PLUS_VECTOR_H