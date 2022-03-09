#include <memory>
#include <utility>
#include <iterator>
#include <map>
#include <cstddef>

template<typename T>
struct storage
{
	alignas(T) std::byte data[sizeof(T)];

	const T* addr() const noexcept
	{ return reinterpret_cast<const T*>(&data); }
	T* addr() noexcept
	{ return reinterpret_cast<T*>(&data); }
};

struct skiplist_node_base {
	typedef int level_size;
	typedef size_t span_type ;
	typedef skiplist_node_base* _Base_ptr;
	typedef const skiplist_node_base* _Const_Base_ptr;

	struct skiplist_node_base * backward;
	static constexpr level_size SKIPLIST_MAXLEVEL=32;
	struct skiplist_node_level{
		struct skiplist_node_base* forward;
		span_type span;
	}level[SKIPLIST_MAXLEVEL];
	skiplist_node_base() = default;
};

template<typename T>
struct skiplist_node :skiplist_node_base
{
	//dont't specialize std::addressof<T> if you dont know what u r doing
	storage<T> data;
	//Why not *val ? cos for extension of std::aligned_storage<T>;
	T* val_ptr()
	{
		return data.addr();
	}
	const T* val_ptr() const
	{
		return data.addr();
	}
};

struct skiplist_node_header
{
	typedef size_t size_type ;
	skiplist_node_base _M_header;
	skiplist_node_base::level_size level_cnt;
	size_type count;

	skiplist_node_header()
	{
		reset();
	}
	void reset() noexcept
	{
		_M_header.level[0].forward = &_M_header;
		_M_header.backward = &_M_header;
		count = 1;
		level_cnt = 1;
	}

};

skiplist_node_base* _Skip_list_increment(const skiplist_node_base* node) throw ();
skiplist_node_base* _Skip_list_decrement(const skiplist_node_base* node) throw ();
template<typename T>
struct _Skip_list_iterator
{
	typedef T value_type;
	typedef T& reference;
	typedef T* pointer;
	typedef _Skip_list_iterator _Self;

	typedef std::bidirectional_iterator_tag iterator_category;
	typedef skiplist_node_base::_Base_ptr _Base_ptr;
	skiplist_node_base* m_node;

	_Skip_list_iterator() = default;
	explicit _Skip_list_iterator(skiplist_node_base* node) noexcept :m_node(node)  {}
	reference operator*()
	{ return *static_cast<skiplist_node<T>*>(m_node)->val_ptr(); }
	pointer operator->()
	{ return static_cast<skiplist_node<T>*>(m_node)->val_ptr(); }
	_Self& operator++()
	{
		m_node=_Skip_list_increment(m_node);
		return *this;
	}
	_Self operator++(int)
	{
		_Self tmp = *this;
		m_node = _Skip_list_increment(m_node);
		return tmp;
	}
	_Self& operator--()
	{
		m_node = _Skip_list_decrement(m_node);
		return *this;
	}
	_Self operator--(int)
	{
		_Self tmp = *this;
		m_node = _Skip_list_decrement(m_node);
		return tmp;
	}

	friend bool operator==(const _Self& lhs, const _Self& rhs) noexcept
	{
		return lhs.m_node == rhs.m_node;
	}
	friend bool operator!=(const _Self& lhs, const _Self& rhs) noexcept
	{ return lhs.m_node!=rhs.m_node; }
};

template<typename T>
struct _Skip_list_const_iterator
{
	typedef T value_type;
	typedef const T& reference;
	typedef const T* pointer;
	typedef _Skip_list_iterator<T> iterator;
	typedef _Skip_list_const_iterator _Self;
	typedef skiplist_node_base::_Const_Base_ptr _Base_ptr;

	typedef std::bidirectional_iterator_tag iterator_category;
	_Base_ptr m_node;

	_Skip_list_const_iterator() = default;
	explicit _Skip_list_const_iterator(_Base_ptr* node) noexcept :m_node(node)  {}
	_Skip_list_const_iterator(const iterator& __it)noexcept:m_node(__it.m_node) {}
	iterator _M_const_cast() const noexcept
	{ return iterator(const_cast<typename iterator::_Base_ptr>(m_node)); }
	reference operator*()
	{ return *static_cast<skiplist_node<T>*>(m_node)->val_ptr(); }
	pointer operator->()
	{ return static_cast<skiplist_node<T>*>(m_node)->val_ptr(); }
	_Self& operator++()
	{
		m_node=_Skip_list_increment(m_node);
		return *this;
	}
	_Self operator++(int)
	{
		_Self tmp = *this;
		m_node = _Skip_list_increment(m_node);
		return tmp;
	}
	_Self& operator--()
	{
		m_node = _Skip_list_decrement(m_node);
		return *this;
	}
	_Self operator--(int)
	{
		_Self tmp = *this;
		m_node = _Skip_list_decrement(m_node);
		return tmp;
	}

	friend bool operator==(const _Self& lhs, const _Self& rhs) noexcept
	{
		return lhs.m_node == rhs.m_node;
	}
	friend bool operator!=(const _Self& lhs, const _Self& rhs) noexcept
	{ return lhs.m_node!=rhs.m_node; }
};

void _Skip_list_insert_and_fix(skiplist_node_base* __x,
		skiplist_node_base* node_info, 
		skiplist_node_header& __header) throw();
void _Skip_list_erase(skiplist_node_base* __x,
		skiplist_node_base* node_info, 
		skiplist_node_header& __header) throw ();

template<typename _Key, typename _Val, typename _KeyOfValue, typename _Compare, typename _Alloc = std::allocator<_Val>>
class _skip_list 
{
	typedef typename std::allocator_traits<_Alloc>::rebind_alloc<skiplist_node<_Val>> _Node_allocator;
      typedef std::allocator_traits<_Node_allocator> _Alloc_traits;
	public:

	typedef size_t size_type;
	typedef _Key key_type;
	typedef _Val value_type;
	typedef _Skip_list_iterator<_Val> iterator;
	typedef _Skip_list_const_iterator<_Val> const_iterator;
	typedef std::map<_Key,iterator> map_type;
	typedef _Alloc allocator_type;

	struct _select_key {
		_skip_list& _M_list;
		_Val at(const _Key& k);
		_Val operator[](const _Key& k);
		size_type rank(const _Key& k);
	};
	struct _select_rank {
		_skip_list& _M_list;
		_Val at(const size_type& r);
		_Val operator[](const size_type& r);
		size_type count(const size_type& r);
		_Key key(const size_type& r);
	};
	protected:
	map_type _M_map;
	/*
	*/
	//template<typename _Compare>
	struct _Skip_list_impl
		: public _Node_allocator
		  ,public skiplist_node_header
	{
		_Compare _M_compare; //skip compile time check temperary 
		typedef size_t size_type;
		_Skip_list_impl() = default;
	};
	_Skip_list_impl _M_impl;
	protected:
	typedef skiplist_node_base* _Base_ptr;
	typedef const skiplist_node_base* _Const_Base_ptr;
	typedef skiplist_node<_Val>* _Link_type;
	typedef const skiplist_node<_Val>* _Const_Link_type;

	_Base_ptr _M_header() noexcept
	{ return &this->_M_impl._M_header; }

	_Link_type
	_M_mbegin() const noexcept
	{ return static_cast<_Link_type>(this->_M_impl._M_header.backward);  }

	_Link_type
	_M_begin() noexcept
	{ return _M_mbegin();  }

	_Const_Link_type
	_M_begin() const noexcept
	{
		return static_cast<_Const_Link_type>
			(this->_M_impl._M_header.backward);
	}

	_Base_ptr
	_M_end() noexcept
	{ return &this->_M_impl._M_header;  }

	_Const_Base_ptr
	_M_end() const noexcept
	{ return &this->_M_impl._M_header;  }
	static const _Key& 
	_S_key(_Const_Link_type __x)
	{
		return _KeyOfValue()(*__x->val_ptr());
	}

	struct _Alloc_node
	{
		_Alloc_node(_skip_list& __s): _M_skiplist(__s)
		{ }

		template<typename _Arg>
	_Link_type
	  operator()(_Arg&& __arg) const
	  { return _M_skiplist._M_create_node(std::forward<_Arg>(__arg)); }
		_skip_list& _M_skiplist;
	};
	public:
	iterator
	begin() noexcept
	{ return iterator(_M_begin()); }

	iterator
	end() noexcept
	{ return iterator(_M_end()); }
	public:
	//struct _select_key select_key;
	//struct _select_rank select_rank;
	_skip_list() = default;
      _Node_allocator&
      _M_get_Node_allocator() noexcept
      { return this->_M_impl; }

      const _Node_allocator&
      _M_get_Node_allocator() const noexcept
      { return this->_M_impl; }

      allocator_type
      get_allocator() const noexcept
      { return allocator_type(_M_get_Node_allocator()); }
	protected:
      _Link_type
      _M_get_node()
      { return _Alloc_traits::allocate(_M_get_Node_allocator(), 1); }

      void
      _M_put_node(_Link_type __p) noexcept
      { _Alloc_traits::deallocate(_M_get_Node_allocator(), __p, 1); }

      template<typename... _Args>
	void
	_M_construct_node(_Link_type __node, _Args&&... __args)
	{
		typedef typename std::remove_pointer<_Link_type>::type node_type;
	  try
	    {
	      ::new(__node) node_type;
	      _Alloc_traits::construct(_M_get_Node_allocator(),
				       __node->val_ptr(),
				       std::forward<_Args>(__args)...);
	    }
	  catch(...)
	    {
	      __node->~node_type();
	      _M_put_node(__node);
		  throw;
	    }
	}

      template<typename... _Args>
	_Link_type
	_M_create_node(_Args&&... __args)
	{
	  _Link_type __tmp = _M_get_node();
	  _M_construct_node(__tmp, std::forward<_Args>(__args)...);
	  return __tmp;
	}

      void
      _M_destroy_node(_Link_type __p) noexcept
      {
		  typedef typename std::remove_pointer<_Link_type>::type node_type;
		_Alloc_traits::destroy(_M_get_Node_allocator(), __p->val_ptr());
		__p->~node_type();
      }

      void
      _M_drop_node(_Link_type __p) noexcept
      {
	_M_destroy_node(__p);
	_M_put_node(__p);
      }
	public:

	template<typename Arg>
	std::pair<iterator, bool>
	_M_insert_unique(Arg&& args);

	std::pair<_Base_ptr, _Base_ptr>
	_M_get_unique_pos(_Base_ptr node_info, const value_type& key);

	template<typename _Arg, typename _Node_gen>
	iterator
	_M_insert_(_Base_ptr node_info, _Arg&& val, _Node_gen node_gen);

	iterator erase(const_iterator __pos)
	{
		const_iterator __res = __pos;
		++__res;
		_M_erase_aux(__pos);
		return __res._M_const_cast();
	}
	//LWG 2059
	iterator erase(iterator __pos)
	{
		iterator __res = __pos;
		++__res;
		_M_erase_aux(__pos);
		return __res;
	}

	iterator 
    erase(const_iterator __first, const_iterator __last)
    {
		_M_erase_aux(__first, __last);
		return __last._M_const_cast();
    }

	void clear()
	{
		_M_erase_from(_M_begin());
		_M_impl.reset();
	}

	private:
		void _M_erase_aux(const_iterator __pos);
		void _M_erase_aux(const_iterator __first, const_iterator __last);
		
};

template<typename _Key, typename _Val, typename _KeyOfValue,typename _Compare, typename _Alloc>
template<typename _Arg>
std::pair<typename _skip_list<_Key, _Val, _KeyOfValue, _Compare, _Alloc>::iterator, bool>
_skip_list<_Key, _Val, _KeyOfValue, _Compare, _Alloc>::_M_insert_unique(_Arg&& arg)
{
	typedef std::pair<iterator, bool> _Res;
	//typedef typename std::remove_pointer<_Base_ptr>::type base_node;

	typename map_type::iterator iter = _M_map.find(_KeyOfValue()(arg));
	if(iter != _M_map.end()) return _Res(iter->second, false);

	skiplist_node_base node_info = skiplist_node_base();
	auto res=_M_get_unique_pos(&node_info, arg);

	if(res.first==nullptr)
	{
		_Alloc_node alloc(*this);
		return _Res(_M_insert_(&node_info,std::forward<_Arg>(arg), alloc),true);
	}else
	{
		return _Res(iterator(res.first),false);
	}

}

template<typename _Key, typename _Val, typename _KeyOfValue,typename _Compare, typename _Alloc>
std::pair<typename _skip_list<_Key, _Val, _KeyOfValue, _Compare, _Alloc>::_Base_ptr, typename _skip_list<_Key, _Val, _KeyOfValue, _Compare, _Alloc>::_Base_ptr> 
_skip_list<_Key, _Val, _KeyOfValue, _Compare, _Alloc>::_M_get_unique_pos(_Base_ptr node_info, const value_type& val)
{
	typedef std::pair<_Base_ptr, _Base_ptr> Res;
	_Base_ptr node = _M_header();
	_Base_ptr end = _M_end();
	//skiplist_node_base::span_type rank[SKIPLIST_MAXLEVEL];

	for(skiplist_node_base::level_size i = _M_impl.level_cnt; i > 0; i--)
	{
		skiplist_node_base::span_type& span = node_info->level[i-1].span;
		span = node_info->level[i].span;
		while(node->level[i-1].forward!=end
				&&_M_impl._M_compare(*static_cast<_Link_type>(node->level[i-1].forward)->val_ptr(),val))
		{
			span += node->level[i-1].span;
			node = static_cast<_Link_type>(node->level[i-1].forward);
		}
		node_info->level[i-1].forward = node;
	}

	if(node->level[0].forward==end||_M_impl._M_compare(val,*static_cast<_Link_type>(node->level[0].forward)->val_ptr()))
	{
		return Res(nullptr,node_info);
	}else
	{
		return Res(node, node_info);
	}
}

template<typename _Key, typename _Val, typename _KeyOfValue,typename _Compare, typename _Alloc>
template<typename _Arg, typename _Node_gen>
typename _skip_list<_Key, _Val, _KeyOfValue, _Compare, _Alloc>::iterator
_skip_list<_Key, _Val, _KeyOfValue, _Compare, _Alloc>::_M_insert_(_Base_ptr node_info, _Arg&& val,  _Node_gen node_gen)
{
	_Link_type __z = node_gen(std::forward<_Arg>(val));

	_Skip_list_insert_and_fix(__z,node_info,_M_impl);
	iterator iter(__z);

	_M_map.insert(typename map_type::value_type(_S_key(__z),iter));
	
	_M_impl.count++;
	return iter;
}

template<typename _Key, typename _Val, typename _KeyOfValue, typename _Compare, typename _Alloc>
void
_skip_list<_Key, _Val, _KeyOfValue, _Compare, _Alloc>::_M_erase_aux(const_iterator __pos)
{
	_Link_type __y = static_cast<_Link_type>(const_cast<_Base_ptr>(__pos.m_node));

	skiplist_node_base node_info = skiplist_node_base();
	auto res=_M_get_unique_pos(&node_info, *__y->val_ptr());
	if(res.first)
	{
		_M_map.erase(_S_key(__y));
		_Skip_list_erase(__y, &node_info, _M_impl);
		_M_drop_node(__y);
		--_M_impl.count;
	}
}

template<typename _Key, typename _Val, typename _KeyOfValue, typename _Compare, typename _Alloc>
void
_skip_list<_Key, _Val, _KeyOfValue, _Compare, _Alloc>::_M_erase_aux(const_iterator __first, const_iterator __last)
{

	_Link_type __y = static_cast<_Link_type>(const_cast<_Base_ptr>(__first.m_node));
	_Link_type __end = static_cast<_Link_type>(const_cast<_Base_ptr>(__first.m_node));

	skiplist_node_base node_info = skiplist_node_base();
	auto res=_M_get_unique_pos(&node_info, *__y->val_ptr());
	if(res.first)
	{
		while(__y!=__end)
		{
			_M_map.erase(_S_key(__y));
			_Skip_list_erase(__y, &node_info, _M_impl);

			_Base_ptr next = __y->level[0].forward;
			_M_drop_node(__y);
			__y = next;
			--_M_impl.count;
		}
	}
}
