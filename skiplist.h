#include "_skiplist.h"

template<typename P>
struct _select1st
{
	typename P::first_type& operator()(P& pair) const noexcept 
	{
		return pair.first;
	}

	const typename P::first_type& operator()(const P& pair) const noexcept
	{
		return pair.first;
	}
};

template<typename P>
struct _select2nd
{
	typename P::second_type& operator()(P& pair) const noexcept 
	{
		return pair.second;
	}

	const typename P::second_type& operator()(const P& pair) const noexcept
	{
		return pair.second;
	}
};

template<typename Pair>
struct _compare2nd
{
	//typedef std::pair<_Key, _Val> value_type;
	bool operator()(const Pair& lhs, const Pair& rhs)
	{
		return lhs.second < rhs.second || (lhs.second == rhs.second && lhs.first < rhs.first);
	}
};

template<typename _Key, typename _Tp, typename _Compare=_compare2nd<std::pair<_Key,_Tp>>, 
typename _Alloc = std::allocator<std::pair<const _Key, _Tp>> >
class skip_list 
{
	public:
		typedef _Key key_type;
		typedef _Tp mapped_type;
		typedef std::pair<const _Key, _Tp> value_type;
		typedef _Compare value_compare;
		typedef _Alloc allocator_type;
		typedef typename std::allocator_traits<allocator_type>::rebind_alloc<value_type> _Pair_alloc_type;
		
		typedef _Skip_list_iterator<value_type> iterator;
		typedef _Skip_list_const_iterator<value_type> const_iterator;

		typedef _skip_list<_Key, value_type, _select1st<value_type>, value_compare, _Pair_alloc_type> skip_list_type;
		typedef size_t size_type;
	protected:
		skip_list_type _M_t;
	public:
		typename skip_list_type::_select_key<mapped_type,_select2nd<value_type>> key;
		typename skip_list_type::_select_rank rank;
		skip_list() :_M_t(), key(_M_t), rank(_M_t) {}
		std::pair<iterator, bool>
		insert(const value_type& __x)
		{ return _M_t._M_insert_unique(__x); }

		iterator
		begin() noexcept
		{ return _M_t.begin(); }

		iterator
		end() noexcept
		{ return _M_t.end(); }

		iterator
		erase(const_iterator iter)
		{ return _M_t.erase(iter); }

		iterator
		erase(const_iterator __first, const_iterator __end)
		{ return _M_t.erase(__first, __end); }

		void clear() noexcept
		{ _M_t.clear(); }

		size_type
		size() const noexcept
		{ return _M_t.size(); }

		bool empty() const noexcept
		{ return _M_t.empty(); }
};
