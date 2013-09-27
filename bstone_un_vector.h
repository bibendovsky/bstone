#ifndef BSTONE_UN_VECTOR_H
#define BSTONE_UN_VECTOR_H


#include <cassert>
#include <cstddef>

#include <algorithm>
#include <limits>
#include <memory>
#include <stdexcept>

#include "bstone_array_const_iterator.h"
#include "bstone_array_iterator.h"


namespace bstone {


//
// A vector of uninitialized POD (plain-old-data) elements.
//
// Differences from the std::vector:
//   - any constructor does not initialize elements with a default value;
//   - on reserving/resizing no constructor/destructor will be called.
//
template<class T,class A = std::allocator<T>>
class UnVector {
public:
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;

    typedef A allocator_type;

    typedef typename bstone::ArrayIt<T> iterator;
    typedef typename bstone::ArrayCIt<T> const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    typedef std::size_t size_type;
    typedef typename iterator::difference_type difference_type;


    explicit UnVector() :
        length_(0),
        capacity_(0),
        allocator_(),
        data_(NULL)
    {
    }

    explicit UnVector(
        const allocator_type& allocator) :
            length_(0),
            capacity_(0),
            allocator_(allocator),
            data_(NULL)
    {
    }

    explicit UnVector(
        size_type length) :
            length_(length),
            capacity_(length),
            allocator_(),
            data_(allocator_.allocate(length))
    {
    }

    UnVector(
        size_type length,
        const_reference value,
        const allocator_type& allocator = allocator_type()) :
            length_(length),
            capacity_(length),
            allocator_(allocator),
            data_(allocator_.allocate(length))
    {
        std::uninitialized_fill(begin(), end(), value);
    }

    UnVector(
        const UnVector& that) :
            length_(that.length_),
            capacity_(that.capacity_),
            allocator_(),
            data_(allocator_.allocate(that.capacity_))
    {
        std::uninitialized_copy(that.begin(), that.end(), data_);
    }

    ~UnVector()
    {
        allocator_.deallocate(data_, capacity_);
    }

    // Replaces the elements of the array with a copy of another array.
    UnVector& operator=(
        UnVector that)
    {
        swap(that);
        return *this;
    }

    // Erases an array and copies the specified elements to
    // the empty array.
    void assign(
        size_type length,
        const_reference value)
    {
        assert(length >= 0);
        resize(length, false);
        std::uninitialized_fill(begin(), end(), value);
    }

    // Erases an array and copies the specified elements to
    // the empty array.
    template<class I>
    void assign(
        I first,
        I last)
    {
        clear();
        assign(first, last, std::iterator_traits<I>::iterator_category());
    }


    // Returns a reference to the element at a specified location in
    // the array.
    reference at(
        size_type index)
    {
        if (index < 0 || index >= length_)
            throw std::out_of_range("index");

        return data_[index];
    }

    // Returns a reference to the element at a specified location in
    // the array.
    const_reference at(
        size_type index) const
    {
        if (index < 0 || index >= length_)
            throw std::out_of_range("index");

        return data_[index];
    }

    // Returns a reference to the last element of the array.
    reference back()
    {
        assert(!empty());
        return data_[length_ - 1];
    }

    // Returns a reference to the last element of the array.
    const_reference back() const
    {
        assert(!empty());
        return data_[length_ - 1];
    }

    // Returns a random-access iterator to the first element in the array.
    iterator begin()
    {
        return data_;
    }

    // Returns a random-access iterator to the first element in the array.
    const_iterator begin() const
    {
        return data_;
    }

    // Returns the number of elements that the array could contain without
    // allocating more storage.
    size_type capacity() const
    {
        return capacity_;
    }

    // Erases the elements of the array.
    void clear()
    {
        length_ = 0;
    }

    // Tests if the array is empty.
    bool empty() const
    {
        return length_ == 0;
    }

    // Returns a random-access iterator that points to the end of
    // the array.
    iterator end()
    {
        return &data_[length_];
    }

    // Returns a random-access iterator that points to the end of
    // the array.
    const_iterator end() const
    {
        return &data_[length_];
    }

    // Removes an element in a vector from specified positions.
    iterator erase(
        iterator pos)
    {
        difference_type index = pos - begin();

        if (index < 0 || index >= length_)
            return end();

        size_type copy_count = length_ - index - 1;

        std::move(
            &data_[index + 1],
            &data_[length_],
            &data_[index]);

        --length_;

        return pos;
    }

    // Removes a range of elements in a vector from specified positions.
    iterator erase(
        iterator first,
        iterator last)
    {
        if (first >= last)
            return end();

        difference_type first_index = first - begin();
        difference_type last_index = last - begin();

        if (first_index < 0)
            first_index = 0;

        if (last_index > length_)
            last_index = length_;

        size_type erase_count = last_index - first_index;

        if (erase_count != length_) {
            std::move(
                &data_[last_index],
                &data_[length_],
                &data_[first_index]);

            length_ -= erase_count;

            return &data_[first_index];
        } else {
            length_ = 0;
            return end();
        }
    }

    // Returns a reference to the first element in a array.
    reference front()
    {
        assert(!empty());
        return data_[0];
    }

    // Returns a reference to the first element in a array.
    const_reference front() const
    {
        assert(!empty());
        return data_[0];
    }

    // Returns a copy of the allocator object used to construct the vector.
    allocator_type get_allocator() const
    {
        return allocator_;
    }

    // Inserts an element into the vector at a specified position.
    iterator insert(
        iterator pos,
        const_reference value)
    {
        difference_type index = pos - begin();

        reserve(length_ + 1);

        std::move(
            &data_[index],
            &data_[length_],
            &data_[index + 1]);

        data_[index] = value;

        ++length_;

        return &data_[index];
    }

    // Inserts a number of elements into the vector at a specified position.
    void insert(
        iterator pos,
        size_type count,
        const_reference value)
    {
        if (count == 0)
            return;

        difference_type index = pos - begin();

        reserve(length_ + count);

        std::move(
            &data_ [index],
            &data_[length_],
            &data_[index + count]);

        std::uninitialized_fill_n(begin() + index, count, value);

        length_ += count;
    }

    // Inserts a number of elements into the vector at a specified position.
    template<class I>
    void insert(
        iterator pos,
        I first,
        I last)
    {
        insert(pos, first, last,
            std::iterator_traits<I>::iterator_category());
    }

    // Returns the maximum length of the array.
    size_type max_size() const
    {
        return std::numeric_limits<size_type>::max() / sizeof(value_type);
    }

    // Deletes the element at the end of the array.
    void pop_back()
    {
        assert(!empty());
        --length_;
    }

    // Add an element to the end of the array.
    void push_back(
        const_reference value)
    {
        resize(length_ + 1);
        back() = value;
    }

    // Returns an iterator to the first element in a reversed array.
    reverse_iterator rbegin()
    {
        return reverse_iterator(end());
    }

    // Returns an iterator to the first element in a reversed array.
    const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(end());
    }

    // Returns an iterator to the end of a reversed array.
    reverse_iterator rend()
    {
        return reverse_iterator(begin());
    }

    // Returns an iterator to the end of a reversed array.
    const_reverse_iterator rend() const
    {
        return const_reverse_iterator(begin());
    }

    // Reserves a minimum length of storage for a array object,
    // allocating space if necessary.
    //
    // NOTE. No constructor or destructor will be called on rellocation.
    void reserve(
        size_type new_capacity)
    {
        if (new_capacity > max_size())
            throw std::length_error("capacity");

        if (capacity_ < new_capacity) {
            capacity_ = new_capacity;

            pointer new_data = allocator_.allocate(capacity_);

            std::uninitialized_copy(
                data_,
                &data_[length_],
                new_data);

            allocator_.deallocate(data_, capacity_);
            data_ = new_data;
        }
    }

    // Specifies a new size for a array.
    //
    // NOTE. No constructor or destructor will be called for
    //       missing/excessive elements.
    void resize(
        size_type new_length)
    {
        if (new_length > capacity_) {
            size_type delta = capacity_ / 2;

            size_type new_capacity = std::max(
                new_length,
                capacity_ + std::min(delta, max_size() - delta));

            reserve(new_capacity);
        }

        length_ = new_length;
    }

    // Returns the number of elements in the array.
    size_type size() const
    {
        return length_;
    }

    // Exchanges the elements of two arrays.
    void swap(
        UnVector& that)
    {
        std::swap(length_, that.length_);
        std::swap(capacity_, that.capacity_);
        std::swap(allocator_, that.allocator_);
        std::swap(data_, that.data_);
    }

    // Returns a reference to the array element at a specified position.
    reference operator[](
        size_type index)
    {
        assert(index >= 0 && index < length_);
        return data_[index];
    }

    // Returns a reference to the array element at a specified position.
    const_reference operator[](
        size_type index) const
    {
        assert(index >= 0 && index < length_);
        return data_[index];
    }

protected:
    size_type length_;
    size_type capacity_;
    pointer data_;
    allocator_type allocator_;


    template<class I>
    void assign(
        I first,
        I last,
        std::input_iterator_tag)
    {
        insert(begin(), first, last);
    }

    template<class I>
    void assign(
        I first,
        I last,
        std::random_access_iterator_tag)
    {
        if (first < last) {
            size_type count = last - first;
            reserve(count);
            std::uninitialized_copy(first, first + count, data_);
            length_ = count;
        }
    }

    template<class I>
    void insert(
        iterator pos,
        I first,
        I last,
        std::input_iterator_tag)
    {
        for ( ; first != last; ++first) {
            pos = insert(pos, *first);
            ++pos;
        }
    }

    template<class I>
    void insert(
        iterator pos,
        I first,
        I last,
        std::random_access_iterator_tag)
    {
        if (first >= last)
            return;

        size_type count = last - first;
        difference_type index = pos - begin();

        reserve(length_ + count);

        std::move(
            &data_[index],
            &data_[length_],
            &data_[index + count]);

        std::uninitialized_copy(first, first + count, &data_[index]);

        length_ += count;
    }
}; // class UnVector


} // namespace bstone


template<class T>
bool operator<(
    const bstone::UnVector<T>& a,
    const bstone::UnVector<T>& b)
{
    return std::lexicographical_compare(
        a.begin(), a.end(), b.begin(), b.end());
}

template<class T>
bool operator>(
    const bstone::UnVector<T>& a,
    const bstone::UnVector<T>& b)
{
    return b < a;
}

template<class T>
bool operator<=(
    const bstone::UnVector<T>& a,
    const bstone::UnVector<T>& b)
{
    return !(b < a);
}

template<class T>
bool operator>=(
    const bstone::UnVector<T>& a,
    const bstone::UnVector<T>& b)
{
    return !(a < b);
}

template<class T>
bool operator==(
    const bstone::UnVector<T>& a,
    const bstone::UnVector<T>& b)
{
    if (a.size() == b.size())
        return std::equal(a.begin(), a.end(), b.begin());
    else
        return false;
}

template<class T>
bool operator!=(
    const bstone::UnVector<T>& a,
    const bstone::UnVector<T>& b)
{
    return !(a == b);
}


#endif // BSTONE_UN_VECTOR_H
