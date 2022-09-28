/* Vaguely STL-ish ringbuffer. Could use some work
 * Written by blindcrone, under WTFPL */
#pragma once
#include <array>
#include <assert.h>
#include <cstddef>
#include <vector>
template <typename T, size_t Max>
class ring {
public:
	using value_type = T;
	using pointer = T*;
	using const_pointer = const T*;
	using reference = T&;
	using const_reference = const T&;
	using size_type = size_t;
	using difference_type = ptrdiff_t;

	class iterator {
	private:
		ring<T, Max>* base;
		size_t idx;
		bool seen_begin;
	public:
		typedef iterator self_type;
		typedef std::forward_iterator_tag iterator_category;
		iterator(ring<T, Max>& b, const size_t i, const bool sb) : base(&b), idx(i), seen_begin(sb) {}
		iterator(ring<T, Max>& b, const size_t i) : iterator(b, i, i) {}
		bool operator==(const self_type& o) const {
			return idx == o.idx && seen_begin == o.seen_begin && base == o.base;
		}
		bool operator!=(const self_type& o) const {
			return !(*this == o);
		}
		reference operator*(){
			return (*base)[idx];
		}
		iterator operator+(size_t i){
			auto clone = *this;
			while(i--)
				++clone;
			return clone;
		}
		iterator& operator++(){
			++idx;
			seen_begin = true;
			return *this;
		}
		iterator operator++(int){
			auto clone = *this;
			++(*this);
			return clone;
		}
	};

	class const_iterator {
	private:
		const ring<T, Max>* base;
		size_t idx;
		bool seen_begin;
	public:
		typedef const_iterator self_type;
		typedef std::forward_iterator_tag iterator_category;
		const_iterator(const ring<T, Max>& b, const size_t i, const bool sb) 
			: base(b), idx(i), seen_begin(sb) {}
		const_iterator(ring<T, Max>& b, const size_t i) : const_iterator(b, i, i) {}
		bool operator==(const self_type& o) const {
			return idx == o.idx && seen_begin == o.seen_begin && base == o.base;
		}
		bool operator!=(const self_type& o) const {
			return !(*this == o);
		}
		const_reference operator*() const {
			return (*base)[idx];
		}
		self_type operator+(size_t i){
			auto clone = *this;
			while(i--)
				++clone;
			return clone;
		}
		self_type& operator++(){
			++idx;
			seen_begin = true;
			return *this;
		}
		self_type operator++(int){
			auto clone = *this;
			++(*this);
			return clone;
		}
	};
	ring(size_type n) : _start(n % Max), _size(std::min(n, Max)) {}
	ring(size_type n, const T& fill) : ring(n) {
		while(n--)
			push_back(fill);
	}
	ring() : ring(0) {}

	reference front() {
		assert(_size != 0);
		return storage[_start];
	}

	reference back() {
		assert(_size != 0);
		return storage[_end() - 1];
	}

	reference at(size_type i) {
		assert(_size > i);
		return storage[_translate_index(i)];
	}

	reference operator[](size_type i) {
		return at(i);
	}

	const_reference front() const {
		assert(_size != 0);
		return storage[_start];
	}

	const_reference back() const {
		assert(_size != 0);
		return storage[_end()];
	}

	const_reference at(size_type i) const {
		assert(_size > i);
		return storage[_translate_index(i)];
	}

	const_reference operator[](size_type i) const {
		return at(i);
	}

	size_type size() const {
		return _size;
	}

	iterator begin() {
		return _size ? iterator(*this, 0) : iterator(*this, 0, true);
	}

	iterator end() {
		return _size ? iterator(*this, _size) : iterator(*this, _size, true);
	}

	const_iterator begin() const {
		return _size ? const_iterator(*this, 0) : const_iterator(*this, 0, true);
	}

	const_iterator end() const {
		return _size ? const_iterator(*this, _size) : const_iterator(*this, _size, true);
	}

	void clear(){
		_size = 0;
	}

	bool empty() const noexcept {
		return !_size;
	}

	bool full() const noexcept {
		return _size == Max;
	}

	void push_back(const value_type& v){
		storage[_end()] = v;
		if(full())
			_start = _translate_index(1);
		else
			++_size;
	}

	void push_back(value_type&& v){
		storage[_end()] = v;
		if(full())
			_start = _translate_index(1);
		else
			++_size;
	}

	template <typename... Args>
	void emplace_back(Args... a){
		storage[_end()] = value_type(a...);
		if(full())
			_start = _translate_index(1);
		else
			++_size;
	}

	void pop_back(){
		assert(_size != 0);
		--_size;
	}

	void push_front(const value_type& v){
		_start = (_start == 0) ? Max - 1 : _start - 1;
		storage[_start] = v;
		if(!full())
			++_size;
	}

	void push_front(value_type&& v){
		_start = (_start == 0) ? Max - 1 : _start - 1;
		storage[_start] = v;
		if(!full())
			++_size;
	}

	void pop_front(){
		assert(_size != 0);
		_start = _translate_index(1);
		--_size;
	}

	template <typename... Args>
	void emplace_front(Args... a){
		_start = (_start == 0) ? Max - 1 : _start - 1;
		storage[_start] = value_type(a...);
		if(!full())
			++_size;
	}
protected:
	std::array<T, Max> storage;
	size_type _start
			, _size
			;
	size_type _translate_index(const size_type i) const {
		return (i + _start) % Max;
	}
	size_type _end() const {
		return _translate_index(_size);
	}
};

