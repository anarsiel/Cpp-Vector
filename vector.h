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
        many_elements_type() : _size(0), _capacity(0), _links(0), _array(nullptr) {}

        many_elements_type(size_t _size, size_t capacity, size_t links_count, T* _array)
                : _size(_size),  _capacity(capacity), _links(links_count), _array(_array) {};

        many_elements_type(many_elements_type &other)
                : _size(other._size), _capacity(other._capacity), _links(other._links), _array(other._array) {}

        ~many_elements_type() {
            for (size_t i = 0; i < _size; ++i) {
                _array[i].~T();
            }
            ::operator delete(_array);
        };

        size_t _size;
        size_t _capacity;
        size_t _links;

        T* _array;
    };

public:
    vector() noexcept : _is_big(false), _empty(true){};

    vector(T const & element) : _is_big(false), _empty(false) {
        new (&buffer.one_element) T(element);
    }

    vector(vector const &other) {
        _is_big = other.is_big();
        _empty = other.empty();

        if (contains_only_one()) {
            new (&buffer.one_element) T(other.buffer.one_element);
        } else if (is_big()) {
            buffer.many_elements = other.buffer.many_elements;
            ++buffer.many_elements->_links;
        }
    }

    ~vector() {
        if (contains_only_one()) {
            buffer.one_element.~T();
        } else if (is_big()) {
            --buffer.many_elements->_links;
            if (get_amount_of_links() == 0) {
                delete(buffer.many_elements);
            }
        }
    }

    template <typename InputIterator>
    vector(InputIterator first, InputIterator last) {
        // todo
    }

    T &operator[](size_t index) {
        if (contains_only_one()) return buffer.one_element;

        make_unique();
        return buffer.many_elements->_array[index];
    }

    T const &operator[](size_t index) const {
        if (contains_only_one()) return buffer.one_element;

        return buffer.many_elements->_array[index];
    }

    T &front() {
        if (contains_only_one()) {
            return buffer.one_element;
        }

        return buffer.many_elements->_array[0];
    }

    T &back() {
        if (contains_only_one()) {
            return buffer.one_element;
        }

        return buffer.many_elements->_array[size() - 1];
    }

    T const &front() const {
        if (contains_only_one()) {
            return buffer.one_element;
        }

        return buffer.many_elements->_array[0];
    }

    T const &back() const {
        if (contains_only_one()) {
            return buffer.one_element;
        }

        return buffer.many_elements->_array[size() - 1];
    }

    void push_back(T const &element) {
        T* element_copy = static_cast<T *>(::operator new(sizeof(T)));
        new(element_copy) T(element);

        if (empty()) {
            new (&buffer.one_element) T(*element_copy);
            _empty = false;
        } else if (contains_only_one()) {
            T *tmp_elem = static_cast<T *>(::operator new(sizeof(T)));
            new (tmp_elem) T(buffer.one_element);

            buffer.one_element.~T();

            T* array_tmp = static_cast<T *>(::operator new(4 * sizeof(T)));
            buffer.many_elements = new many_elements_type(2, 4, 1, array_tmp);

            new (&buffer.many_elements->_array[0]) T(*tmp_elem);
            delete(tmp_elem);

            new (&buffer.many_elements->_array[1]) T(*element_copy);

            _is_big = true;
        } else {
            make_unique();
            ensure_capacity();

            new(&buffer.many_elements->_array[size()]) T(*element_copy);
            buffer.many_elements->_size++;
        }

        delete(element_copy);
    }

    void pop_back() {
        if (contains_only_one()) {
            buffer.one_element.~T();
            _empty = true;
            return;
        }

        if (size() == 2) {
            make_unique();

            T* tmp = static_cast<T*> (::operator new(sizeof(T)));
            new (tmp) T(buffer.many_elements->_array[0]);

//            buffer.many_elements->_array[0].~T();
//            buffer.many_elements->_array[1].~T();

            delete(buffer.many_elements);

            new (&buffer.one_element) T(*tmp);
            delete(tmp);
            _is_big = false;
            return;
        }

        make_unique();
        ensure_capacity();

        (buffer.many_elements->_array[size() - 1]).~T();
        buffer.many_elements->_size--;
    }

    T* data() {
        if (empty())
            return nullptr;

        if (contains_only_one()) {
            return &buffer.one_element;
        }

        return buffer.many_elements->_array;
    }

    T const * data() const {
        if (empty())
            return nullptr;

        if (contains_only_one()) {
            return &buffer.one_element;
        }

        return buffer.many_elements->_array;
    }

    bool empty() const { return _empty; }

    size_t size() const {
        if (empty()) return 0;
        if (!is_big() && !empty()) return 1;

        return buffer.many_elements->_size;
    }

    size_t capacity() const {
        if (empty()) return 0;
        if (contains_only_one()) return 1;

        return get_capacity();
    }

    void clear() {}

    vector& operator=(vector const &other) {
        while (size() > 0) {
            pop_back();
        }

        _is_big = other._is_big;
        _empty = other._empty;
        if (other.contains_only_one()) {
            buffer.one_element = other.buffer.one_element;
        } else if (other.is_big()) {
            buffer.many_elements = other.buffer.many_elements;
            ++buffer.many_elements->_links;
        }

        return *this;
    }

private:
    bool _is_big, _empty;

    union U {
        U() {}
        ~U() {};

        many_elements_type* many_elements;
        T one_element;
    } buffer;

    bool is_big() const {
        return _is_big;
    }

    bool contains_only_one() const {
        return !is_big() && !empty();
    }

    size_t get_capacity() const { return buffer.many_elements->_capacity; }
    size_t get_amount_of_links() const { return buffer.many_elements->_links; }
    bool unique() { return !is_big() || (is_big() && (get_amount_of_links() == 1)); }

    void make_unique() {
        if (unique()) return;

        auto tmp = static_cast<T *>(::operator new(get_capacity() * sizeof(T)));
        buffer.many_elements->_links--;

        for (size_t i = 0; i < size(); ++i) {
            new(tmp + i) T(buffer.many_elements->_array[i]);
        }

        auto tmp_buffer = new many_elements_type(size(), capacity(), get_amount_of_links(), tmp);

        buffer.many_elements = tmp_buffer;
    }

    // object must be unique
    void ensure_capacity() {
        if (!is_big()) return;

        if (size() == get_capacity()) {
            auto tmp_array = static_cast<T *>(::operator new(2 * get_capacity() * sizeof(T)));

            size_t tmp_size_var = size();
            size_t tmp_capacity_var = capacity();
            for (size_t i = 0; i < tmp_size_var; ++i) {
                new(tmp_array + i) T(buffer.many_elements->_array[i]);
                buffer.many_elements->_array[i].~T();
            }
            ::operator delete(buffer.many_elements->_array);

            buffer.many_elements->_array = tmp_array;
            buffer.many_elements->_size = tmp_size_var;
            buffer.many_elements->_capacity = 2 * tmp_capacity_var;
            buffer.many_elements->_links = 1;

        }
    }
};

#endif //C_PLUS_PLUS_VECTOR_H