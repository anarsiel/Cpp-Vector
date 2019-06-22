#ifndef C_PLUS_PLUS_VECTOR_H
#define C_PLUS_PLUS_VECTOR_H

//
// Created by Anarsiel on 2019-06-15.
//

#include <cstddef>
#include <memory>
#include <string.h>

template<typename T>
struct vector {

    typedef T value_type;

private:
    struct many_elements_type {
        many_elements_type() : _size(0), _capacity(0), _links(0), _array(nullptr) {}

        many_elements_type(size_t _size, size_t capacity, size_t links_count, T *_array)
                : _size(_size), _capacity(capacity), _links(links_count), _array(_array) {};

        many_elements_type(many_elements_type &other)
                : _size(other._size), _capacity(other._capacity), _links(other._links), _array(other._array) {}

        ~many_elements_type() {
            for (size_t i = 0; i != _size; ++i) {
                _array[i].~T();
            }
            ::operator delete(_array);
        };

        size_t _size;
        size_t _capacity;
        size_t _links;

        T *_array;
    };

public:
    vector() noexcept : _is_big(false), _empty(true) {};

    vector(T const &element) : _is_big(false), _empty(false) {
        new(&buffer.one_element) T(element);
    }

    vector(vector const &other) {
        _is_big = other.is_big();
        _empty = other.empty();

        if (contains_only_one()) {
            new(&buffer.one_element) T(other.buffer.one_element);
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
                delete (buffer.many_elements);
            }
        }
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
        if (empty()) {
            new(&buffer.one_element) T(element);
            _empty = false;
        } else if (!is_big()) {
            T *element_copy = static_cast<T *>(::operator new(sizeof(T)));

            try {
                new(element_copy) T(element);
            } catch (...) {
                ::operator delete(element_copy);
                throw;
            }

            T* tmp_elem;
            try {
                tmp_elem = new T(buffer.one_element);
            } catch (...) {
                delete(element_copy);
                throw;
            }

            T* array_tmp;
            try {
                array_tmp = static_cast<T *>(::operator new(4 * sizeof(T)));
            } catch (...) {
                delete(element_copy);
                delete(tmp_elem);
                throw;
            }

            many_elements_type* met_p;

            try {
                met_p = new many_elements_type(2, 4, 1, array_tmp);
            } catch (...) {
                delete(tmp_elem);
                delete(element_copy);
                ::operator delete(array_tmp);
                throw;
            }

            try {
                new(met_p->_array) T(*tmp_elem);
            } catch (...) {
                met_p->_size = 0;
                delete(met_p);

                delete(tmp_elem);
                delete(element_copy);
                throw;
            }

            try {
                new(met_p->_array + 1) T(*element_copy);
            } catch (...) {
                met_p->_size = 1;
                delete(met_p);

                delete(tmp_elem);
                delete(element_copy);
                throw;
            }

            buffer.one_element.~T();

            buffer.many_elements = met_p;

            delete (tmp_elem);
            delete (element_copy);

            _is_big = true;
        } else {
            bool need_to_copy = (buffer.many_elements->_array <= &element && &element <= buffer.many_elements->_array + size());

            T *element_copy;
            if (need_to_copy)
                element_copy = new T(element);

            make_unique();

            try {
                ensure_capacity();
            } catch (...) {
                if (need_to_copy)
                    delete(element_copy);
                throw;
            }

            try {
                if (need_to_copy) {
                    new(&buffer.many_elements->_array[size()]) T(*element_copy);
                } else {
                    new(buffer.many_elements->_array + size()) T(element);
                }
            } catch (...) {
                if (need_to_copy)
                    delete(element_copy);

                throw;
            }
            buffer.many_elements->_size++;

            if (need_to_copy)
                delete (element_copy);
        }
    }

    void pop_back() {
        if (!is_big()) {
            if (!empty()) buffer.one_element.~T();
            _empty = true;
        } else {
            buffer.many_elements->_size--;
            if (!empty()) buffer.many_elements->_array[size()].~T();
            if (buffer.many_elements->_size == 0) {
                _empty = true;
            }
        }
    }

    void reserve(size_t new_capacity) {
        if (new_capacity < capacity()) return;

        make_unique();

        if (empty()) {
            if (new_capacity > 1) {
                T* tmp_array = static_cast<T *>(::operator new(new_capacity * sizeof(T)));

                try {
                    buffer.many_elements = new many_elements_type(0, new_capacity, 1, tmp_array);
                } catch (...) {
                    ::operator delete(tmp_array);
                    throw;
                }


                _empty = false;
                _is_big = true;
            }
            return;
        }

        if (contains_only_one()) {
            _empty = false;
            _is_big = true;

            T *tmp_elem = static_cast<T *>(::operator new(sizeof(T)));
            new(tmp_elem) T(buffer.one_element);

            buffer.one_element.~T();

            T *array_tmp = static_cast<T *>(::operator new(new_capacity * sizeof(T)));
            buffer.many_elements = new many_elements_type(1, new_capacity, 1, array_tmp);

            new(&buffer.many_elements->_array[0]) T(*tmp_elem);
            delete (tmp_elem);

            _is_big = true;
            return;
        }

        make_unique();

        auto tmp_array = static_cast<T *>(::operator new(new_capacity * sizeof(T)));

        size_t tmp_size_var = size();
        size_t tmp_capacity_var = new_capacity;

        for (size_t i = 0; i != tmp_size_var; ++i) {
            try {
                new(tmp_array + i) T(buffer.many_elements->_array[i]);
            } catch (...) {
                for (size_t j = 0; j < i; ++j) {
                    tmp_array[i - j - 1].~T();
                }
                ::operator delete(tmp_array);
                throw;
            }
        }

        for (size_t i = 0; i != tmp_size_var; ++i) {
            buffer.many_elements->_array[i].~T();
        }
        ::operator delete(buffer.many_elements->_array);

        buffer.many_elements->_array = tmp_array;
        buffer.many_elements->_size = tmp_size_var;
        buffer.many_elements->_capacity = tmp_capacity_var;
        buffer.many_elements->_links = 1;
    }

    void shrink_to_fit() {
        if (!is_big()) return;

        make_unique();

        auto tmp_array = static_cast<T *>(::operator new(size() * sizeof(T)));

        size_t tmp_size_var = size();
        size_t tmp_capacity_var = size();
        for (size_t i = 0; i != tmp_size_var; ++i) {
            new(tmp_array + i) T(buffer.many_elements->_array[i]);
            buffer.many_elements->_array[i].~T();
        }
        ::operator delete(buffer.many_elements->_array);

        buffer.many_elements->_array = tmp_array;
        buffer.many_elements->_size = tmp_size_var;
        buffer.many_elements->_capacity = tmp_capacity_var;
        buffer.many_elements->_links = 1;
    }

    void resize(size_t _new_size) {
        while (size() < _new_size) {
            push_back(T());
        }

        while (size() > _new_size) {
            pop_back();
        }
    }

    void resize(size_t _new_size, T &element) {
        while (size() < _new_size) {
            push_back(element);
        }

        while (size() > _new_size) {
            pop_back();
        }
    }

    T *data() {
        if (empty())
            return nullptr;

        if (contains_only_one()) {
            return &buffer.one_element;
        }

        return buffer.many_elements->_array;
    }

    T const *data() const {
        if (empty())
            return nullptr;

        if (contains_only_one()) {
            return &buffer.one_element;
        }

        return buffer.many_elements->_array;
    }

    bool empty() const noexcept { return _empty; }

    size_t size() const noexcept {
        if (empty()) return 0;
        if (!is_big() && !empty()) return 1;

        return buffer.many_elements->_size;
    }

    size_t capacity() const {
        if (empty()) return 0;
        if (contains_only_one()) return 1;

        return get_capacity();
    }

    void clear() {
        while (size() != 0) {
            pop_back();
        }
    }

    vector &operator=(vector const &other) {
        vector tmp(other);

        swap(tmp, *this);
        return *this;
    }

    friend bool operator==(vector const &a, vector const &b) {
        if (a.size() != b.size()) return false;

        for (size_t i = 0; i != a.size(); ++i) {
            if (a[i] != b[i]) return false;
        }

        return true;
    }

    friend bool operator!=(vector const &a, vector const &b) {
        return !(a == b);
    }

    friend bool operator<(vector const &a, vector const &b) {
        for (size_t i = 0; i != std::min(a.size(), b.size()); ++i) {
            if (a[i] < b[i]) return true;
            if (a[i] > b[i]) return false;
        }

        return a.size() < b.size();
    }

    friend bool operator<=(vector const &a, vector const &b) {
        for (size_t i = 0; i != std::min(a.size(), b.size()); ++i) {
            if (a[i] < b[i]) return true;
            if (a[i] > b[i]) return false;
        }

        return a.size() <= b.size();
    }

    friend bool operator>(vector const &a, vector const &b) {
        return !(a <= b);
    }

    friend bool operator>=(vector const &a, vector const &b) {
        return !(a < b);
    }

    friend void swap(vector &a, vector &b) {
        bool f_a_1 = a._empty;
        bool f_a_2 = a._is_big;
        bool f_b_1 = b._empty;
        bool f_b_2 = b._is_big;

        if (a.contains_only_one() && b.contains_only_one()) {
            std::swap(a.buffer.one_element, b.buffer.one_element);
        } else if (a.is_big() && b.is_big()) {
            many_elements_type *tmp = b.buffer.many_elements;
            b.buffer.many_elements = a.buffer.many_elements;
            a.buffer.many_elements = tmp;
        } else if (a.contains_only_one() && b.empty()) {
            b.push_back(a[0]);
            a.clear();
        } else if (a.contains_only_one() && b.is_big()) {
            auto tmp = new T(a.buffer.one_element);
            auto tmp_m_e = b.buffer.many_elements;

            try {
                new (&b.buffer.one_element) T(*tmp);
            } catch (...) {
                delete(tmp);
                throw;
            }
            a.buffer.one_element.~T();
            a.buffer.many_elements = tmp_m_e;
            delete(tmp);
        } else if (b.contains_only_one() && a.is_big()) {
            auto tmp = new T(b.buffer.one_element);
            auto tmp_m_e = a.buffer.many_elements;

            try {
                new(&a.buffer.one_element) T(*tmp);
            } catch (...) {
                delete (tmp);
                throw;
            }
            b.buffer.one_element.~T();
            b.buffer.many_elements = tmp_m_e;
            delete (tmp);
        } else if (b.contains_only_one() && a.empty()) {
            a.push_back(b[0]);
            b.clear();
        } else if (a.is_big() && b.empty()) {
            b.buffer.many_elements = a.buffer.many_elements;
            a.clear();
        } else if (b.is_big() && a.empty()) {
            a.buffer.many_elements = b.buffer.many_elements;
        }

        a._empty = f_b_1;
        a._is_big = f_b_2;
        b._empty = f_a_1;
        b._is_big = f_a_2;
    }

    template<typename V>
    struct vector_iterator : std::iterator<std::random_access_iterator_tag, V> {

        friend vector;

        vector_iterator() = default;

        ~vector_iterator() = default;

        template <typename X>
        vector_iterator(X* _data) : _data(_data) {}

        template <typename X>
        vector_iterator(vector_iterator<X> const &other) : _data(other._data) {}

        template <typename X>
        bool operator==(vector_iterator<X> const &other) const {
            return _data == other._data;
        }

        template <typename X>
        bool operator!=(vector_iterator<X> const &other) const {
            return _data != other._data;
        }

        template <typename X>
        bool operator<=(vector_iterator<X> const &other) const {
            return _data <= other._data;
        }

        template <typename X>
        bool operator>=(vector_iterator<X> const &other) const {
            return _data >= other._data;
        }

        template <typename X>
        bool operator<(vector_iterator<X> const &other) const {
            return _data < other._data;
        }

        template <typename X>
        bool operator>(vector_iterator<X> const &other) const {
            return _data > other._data;
        }

        vector_iterator &operator++() {
            ++_data;
            return *this;
        }

        const vector_iterator operator++(int) {
            vector_iterator tmp = vector_iterator(*this);
            ++(*this);
            return tmp;
        }

        vector_iterator &operator--() {
            --_data;
            return *this;
        }

        const vector_iterator operator--(int) {
            vector_iterator tmp = vector_iterator(*this);
            --(*this);
            return tmp;
        }

        vector_iterator &operator+=(size_t x) {
            _data += x;
            return *this;
        }

        vector_iterator &operator-=(size_t x) {
            _data -= x;
            return *this;
        }

        vector_iterator operator+(size_t x) {
            vector_iterator tmp = vector_iterator(*this);
            tmp += x;
            return tmp;
        }

        vector_iterator operator-(size_t x) {
            vector_iterator tmp = vector_iterator(*this);
            tmp -= x;
            return tmp;
        }

        V& operator*() {
            return *_data;
        }

        V* operator->() {
            return _data;
        }

    private:
        V* _data;
    };

    typedef vector_iterator<T> iterator;
    typedef vector_iterator<T const> const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    template<typename InputIterator>
    vector(InputIterator first, InputIterator last) {
        for (auto it = first; it != last; ++it) {
            push_back(*it);
        }
    }

    template<typename InputIterator>
    void assign(InputIterator first, InputIterator last) {
        clear();
        for (auto it = first; it != last; ++it) {
            push_back(*it);
        }
    }

    iterator begin() {
        if (is_big())
            return iterator(buffer.many_elements->_array);

        if (contains_only_one())
            return iterator(&buffer.one_element);

        return iterator(data());
    }

    iterator end() {
        if (is_big()) {
            return iterator(buffer.many_elements->_array + size());
        } else if (contains_only_one()) {
            return iterator(&buffer.one_element + 1);
        }

        return iterator(data());
    }

    const_iterator begin() const {
        if (is_big()) {
            return const_iterator(buffer.many_elements->_array);
        } else if (contains_only_one()) {
            return const_iterator(&buffer.one_element);
        }

        return const_iterator(data());
    }

    const_iterator end() const {
        if (is_big()) {
            return const_iterator(buffer.many_elements->_array + size());
        } else if (contains_only_one()) {
            return const_iterator(&buffer.one_element + 1);
        }

        return const_iterator(data());
    }

    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    reverse_iterator rend() {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator rend() const {
        return const_reverse_iterator(begin());
    }

    void insert(iterator it, T const &x) {
        vector tmp;
        for (auto it1 = begin(); it1 != it; ++it1) {
            tmp.push_back(*it1);
        }

        tmp.push_back(x);

        for (auto it1 = it; it1 != end();  ++it1) {
            tmp.push_back(*it1);
        }

        swap(*this, tmp);
    }

    void erase(iterator it1, iterator it2) {
        vector tmp;

        for (auto it = begin(); it != it1; ++it) {
            tmp.push_back(*it);
        }

        for (auto it = it2; it != end(); ++it) {
            tmp.push_back(*it);
        }

        swap(*this, tmp);
    }

    void erase(iterator it1) {
        erase(it1, it1 + 1);
    }

private:
    bool _is_big, _empty;

    union U {
        U() {}

        ~U() {};

        many_elements_type *many_elements;
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

        for (size_t i = 0; i != size(); ++i) {
            try {
                new(tmp + i) T(buffer.many_elements->_array[i]);
            } catch (...) {
                for (size_t j = 0; j < i; ++j) {
                    tmp[i - j - 1].~T();
                }
                ::operator delete(tmp);
                buffer.many_elements->_links++;
                throw;
            }
        }

        many_elements_type* tmp_buffer;
        try {
            tmp_buffer = new many_elements_type(size(), capacity(), get_amount_of_links(), tmp);
        } catch (...) {
            for (size_t j = 0; j < size(); ++j) {
                tmp[size() - j - 1].~T();
            }
            ::operator delete(tmp);
            buffer.many_elements->_links++;
            throw;
        }

        buffer.many_elements = tmp_buffer;
    }

    void ensure_capacity() {
        if (!is_big()) return;

        if (size() == get_capacity()) {
            T* tmp_array;
            tmp_array = static_cast<T *>(::operator new(2 * get_capacity() * sizeof(T)));

            size_t tmp_size_var = size();
            size_t tmp_capacity_var = capacity();
            for (size_t i = 0; i != tmp_size_var; ++i) {
                try {
                    new(tmp_array + i) T(buffer.many_elements->_array[i]);
                } catch (...) {
                    for (size_t j = 0; j < i; ++j) {
                        tmp_array[i - j - 1].~T();
                    }
                    ::operator delete(tmp_array);
                    throw;
                }
            }

            for (size_t i = 0; i != tmp_size_var; ++i) {
                buffer.many_elements->_array[i].~T();
            }
            ::operator delete(buffer.many_elements->_array);
//            std::uninitialized_copy(buffer.many_elements->_array, buffer.many_elements->_array + size(), tmp_array);

            buffer.many_elements->_array = tmp_array;
            buffer.many_elements->_size = tmp_size_var;
            buffer.many_elements->_capacity = 2 * tmp_capacity_var;
            buffer.many_elements->_links = 1;
        }
    }
};

#endif //C_PLUS_PLUS_VECTOR_H