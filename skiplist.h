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

template<typename Pair>
struct _compare2nd
{
	//typedef std::pair<_Key, _Val> value_type;
	bool operator()(const Pair& lhs, const Pair& rhs)
	{
		return lhs.second < rhs.second;
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

		typedef _skip_list<_Key, value_type, _select1st<value_type>, value_compare, _Pair_alloc_type> skip_list_type;
	protected:
		skip_list_type _M_t;
	public:
		std::pair<iterator, bool>
		insert(const value_type& __x)
		{ return _M_t._M_insert_unique(__x); }

		iterator
		begin() noexcept
		{ return _M_t.begin(); }

		iterator
		end() noexcept
		{ return _M_t.end(); }

};
