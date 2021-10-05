#include <avakar/container_of.h>
#include <algorithm>

namespace avakar {
namespace intrusive {

struct list_node
{
	list_node() noexcept
	{
		this->_reset();
	}

	bool attached() const noexcept
	{
		return _next != this;
	}

	void detach() noexcept
	{
		_prev->_next = _next;
		_next->_prev = _prev;
		this->_reset();
	}

	~list_node()
	{
		this->detach();
	}

	list_node(list_node && o) noexcept
	{
		_next = &o;
		_prev = o._prev;
		_next->_prev = this;
		_prev->_next = this;

		o.detach();
	}

	list_node & operator=(list_node && o) noexcept
	{
		list_node & anchor = *o._next;
		o.detach();
		this->_attach_before(anchor);
		return *this;
	}

private:
	void _attach_before(list_node & anchor) noexcept
	{
		_prev->_next = _next;
		_next->_prev = _prev;

		_next = &anchor;
		_prev = anchor._prev;
		_next->_prev = this;
		_prev->_next = this;
	}

	void _reset() noexcept
	{
		_prev = this;
		_next = this;
	}

	template <typename T, list_node T::* member>
	friend struct list;

	list_node * _prev;
	list_node * _next;
};

template <typename T, list_node T::* member>
struct list
{
	list() noexcept = default;

	bool empty() const noexcept
	{
		return _head._next == &_head;
	}

	void clear() noexcept
	{
		list_node * cur = _head._next;
		while (cur != &_head)
		{
			list_node * p = cur;
			cur = cur->_next;
			p->_reset();
		}

		_head._reset();
	}

	std::size_t size() const noexcept
	{
		return std::distance(this->begin(), this->end());
	}

	void push_front(T & v) noexcept
	{
		(v.*member)._attach_after(_head);
	}

	void pop_front() noexcept
	{
		_head._next->detach();
	}

	T & front() const noexcept
	{
		return *container_of(_head._next, member);
	}

	void push_back(T & v) noexcept
	{
		(v.*member)._attach_before(_head);
	}

	void pop_back() noexcept
	{
		_head._prev->detach();
	}

	T & back() const noexcept
	{
		return *container_of(_head._prev, member);
	}

	struct const_iterator
	{
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = T const;
		using difference_type = std::ptrdiff_t;
		using pointer = T const *;
		using reference = T const &;

		const_iterator() noexcept
		{
		}

		const_iterator(T const & v) noexcept
			: _cur(&(v.*member))
		{
		}

		T const & operator*() const noexcept
		{
			return *container_of(_cur, member);
		}

		T const * operator->() const noexcept
		{
			return container_of(_cur, member);
		}

		const_iterator & operator++() noexcept
		{
			_cur = _cur->_next;
			return *this;
		}

		const_iterator operator++(int) noexcept
		{
			const_iterator r = *this;
			++*this;
			return r;
		}

		const_iterator & operator--() noexcept
		{
			_cur = _cur->_prev;
			return *this;
		}

		const_iterator operator--(int) noexcept
		{
			const_iterator r = *this;
			--*this;
			return r;
		}

		friend bool operator==(const_iterator lhs, const_iterator rhs) noexcept
		{
			return lhs._cur == rhs._cur;
		}

		friend bool operator!=(const_iterator lhs, const_iterator rhs) noexcept
		{
			return !(lhs == rhs);
		}

	private:
		friend list;
		friend struct iterator;

		explicit const_iterator(list_node const * cur) noexcept
			: _cur(cur)
		{
		}

		list_node const * _cur;
	};

	struct iterator
	{
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = T;
		using difference_type = std::ptrdiff_t;
		using pointer = T *;
		using reference = T &;

		iterator() noexcept
		{
		}

		iterator(T & v) noexcept
			: _cur(&(v.*member))
		{
		}

		operator const_iterator() const noexcept
		{
			return const_iterator(_cur);
		}

		T & operator*() const noexcept
		{
			return *container_of(_cur, member);
		}

		T * operator->() const noexcept
		{
			return container_of(_cur, member);
		}

		iterator & operator++() noexcept
		{
			_cur = _cur->_next;
			return *this;
		}

		iterator operator++(int) noexcept
		{
			iterator r = *this;
			++*this;
			return r;
		}

		iterator & operator--() noexcept
		{
			_cur = _cur->_prev;
			return *this;
		}

		iterator operator--(int) noexcept
		{
			iterator r = *this;
			--*this;
			return r;
		}

		friend bool operator==(iterator lhs, iterator rhs) noexcept
		{
			return lhs._cur == rhs._cur;
		}

		friend bool operator!=(iterator lhs, iterator rhs) noexcept
		{
			return !(lhs == rhs);
		}

	private:
		friend list;

		explicit iterator(list_node * cur) noexcept
			: _cur(cur)
		{
		}

		list_node * _cur;
	};

	iterator insert(iterator where, T & v) noexcept
	{
		list_node * entry = &(v.*member);

		entry->_attach_before(*where._cur);
		return iterator(entry);
	}

	iterator erase(iterator where) noexcept
	{
		iterator r = where;
		++r;
		where._cur->detach();
		return r;
	}

	const_iterator cbegin() const noexcept
	{
		return const_iterator(_head._next);
	}

	const_iterator cend() const noexcept
	{
		return const_iterator(&_head);
	}

	const_iterator begin() const noexcept
	{
		return this->cbegin();
	}

	const_iterator end() const noexcept
	{
		return this->cend();
	}

	iterator begin() noexcept
	{
		return iterator(_head._next);
	}

	iterator end() noexcept
	{
		return iterator(&_head);
	}

	~list()
	{
		this->clear();
	}

	list(list && o) noexcept = default;
	list & operator=(list && o) noexcept = default;

	list(list const & o) noexcept = delete;

private:
	list_node _head;
};

}
}

#pragma once
