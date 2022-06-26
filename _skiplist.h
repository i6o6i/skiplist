#include <memory>
#include <utility>
#include <iterator>
#include <map>
#include <cstddef>
#include <stdexcept>
#include <cstring>
#include <type_traits>

template<typename T>
struct storage
{
	typename std::aligned_storage<sizeof(T), alignof(T)>::type data;
	//alignas(T) std::byte data[sizeof(T)];

	const T* addr() const noexcept
	{ return reinterpret_cast<const T*>(&data); }
	T* addr() noexcept
	{ return reinterpret_cast<T*>(&data); }
};

struct skiplist_node_base {
	typedef int level_size;
	//sum of span_type should be less than 2<<sizeof(size_t) -1 see _M_get_unique_pos
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
		count = 0;
		level_cnt = 1;
	}

	const skiplist_node_base* end() const noexcept
	{ return &_M_header; }

	skiplist_node_base* end() noexcept
	{ return &_M_header; }

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
	_Skip_list_const_iterator(_Base_ptr node) noexcept :m_node(node)  {}
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
void _Skip_list_insert_and_fix(skiplist_node_base* __x,
		skiplist_node_base* node_info, 
		skiplist_node_header& __header, bool *IsForwardDups, bool IsBackwardDup) throw ();
void _Skip_list_erase(skiplist_node_base* __x,
		skiplist_node_base* node_info, 
		skiplist_node_header& __header) throw ();
void _Skip_list_erase(skiplist_node_base* __x,
		skiplist_node_base* node_info, 
		skiplist_node_header& __header, bool IsDup) throw ();
skiplist_node_base* _Skip_list_get_rank(skiplist_node_header& header, skiplist_node_header::size_type rank) throw();

template<typename _Key, typename _Val, typename _KeyOfValue, typename _Compare, bool IsJuxtaposed, typename _Alloc = std::allocator<_Val>>
class _skip_list 
{
	typedef typename std::allocator_traits<_Alloc>::template rebind_alloc<skiplist_node<_Val>> _Node_allocator;
      typedef std::allocator_traits<_Node_allocator> _Alloc_traits;
	public:

	typedef typename skiplist_node_header::size_type size_type;
	typedef _Key key_type;
	typedef _Val value_type;
	typedef _Skip_list_iterator<_Val> iterator;
	typedef _Skip_list_const_iterator<_Val> const_iterator;
	typedef std::map<_Key,iterator> map_type;
	typedef _Alloc allocator_type;

	protected:
	struct _Skip_list_impl
		: public _Node_allocator
		  ,public skiplist_node_header
	{
		//strict relation
		struct _Val_Compare :std::binary_function<value_type, value_type, bool>
		{
			bool operator()(const value_type& lhs, const value_type& rhs) const
			{
				return _Compare()(lhs,rhs) || (!_Compare()(rhs,lhs) && std::memcmp(&lhs,&rhs,sizeof(value_type)) < 0);
			}
		};
	   _Val_Compare	_M_strict_compare;
		_Compare _M_compare; //skip compile time check temperary 
		typedef size_t size_type;
		_Skip_list_impl() = default;
	};
	public:
	template<typename _Up_Val, typename _ValOf>
	struct _select_key {
		typedef _ValOf _Up_ValOf;
		_skip_list& _M_list;
		_select_key(_skip_list& list) noexcept:_M_list(list) { }
		_Up_Val& at(const _Key& k);
		const _Up_Val& at(const _Key& k) const;
		iterator find(const _Key& k);
		const_iterator find(const _Key& k) const;
		//_Up_Val operator[](const _Key& k);
		//
		//template<typename std::enable_if<IsJuxtaposed,bool>::type abool = true>
		//bool
		template<bool _IsJuxtaposed=IsJuxtaposed>
		typename std::enable_if<_IsJuxtaposed,size_type>::type 
		rankofkey(const _Key& k) const;

		template<bool _IsJuxtaposed=IsJuxtaposed>
		typename std::enable_if<!_IsJuxtaposed,size_type>::type rankofkey(const _Key& k) const;
	};
	struct _select_rank {
		_skip_list& _M_list;
		_select_rank(_skip_list& list) noexcept:_M_list(list) { }
		_Val& at(size_type n);
		const _Val& at(size_type n) const;
		iterator find(size_type n);
		const_iterator find(size_type n) const;
		//_Val operator[](const size_type& r);
		size_type count(size_type r);
		_Key keyofrank(size_type n);
		template<bool _IsJuxtaposed = IsJuxtaposed>
		typename std::enable_if<_IsJuxtaposed,std::pair<iterator, iterator>>::type
		equal_range(size_type n);
	};
	protected:
	_Skip_list_impl _M_impl;
	map_type _M_map;
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
	{ return _M_impl.end();  }

	_Const_Base_ptr
	_M_end() const noexcept
	{ return _M_impl.end();  }
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
	//_skip_list() : _M_impl(), _M_map(), key(_M_impl), rank(_M_impl) {}
	  _skip_list()  = default;

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

	void
	clear() noexcept
	{
		_M_erase_aux(_M_begin(),_M_end());
		_M_impl.reset();
	}

	size_type
	size() const noexcept
	{
		return _M_map.size();
	}
	bool empty() const noexcept
	{
		return _M_map.empty();
	}

	private:
	/*
		template<bool _IsJuxtaposed = IsJuxtaposed>
		struct hepler {
			_Skip_list_impl& _M_impl;
			hepler(_Skip_list_impl& _impl) { _M_impl = _impl; }
			template <typename std::enable_if<IsJuxtaposed>::type>
			bool IsForwardDup(skiplist_node_base* node,skiplist_node_base::level_size level);
			template <typename std::enable_if<IsJuxtaposed>::type>
			bool IsBackwardDup(skiplist_node_base* node);
			template<typename _Arg, typename _Node_gen>
			typename _skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::iterator _M_insert_(_Base_ptr node_info, _Arg&& val, _Node_gen node_gen);
			void _M_erase_aux(const_iterator __pos) ;
			void _M_erase_aux(const_iterator __first, const_iterator __last) ;
		};
		struct hepler<IsJuxtaposed> _M_helper;
		*/

		template<bool _IsJuxtaposed = IsJuxtaposed>
		typename std::enable_if<_IsJuxtaposed,bool>::type IsForwardDup(skiplist_node_base* node, skiplist_node_base::level_size level);
		
		template<bool _IsJuxtaposed = IsJuxtaposed>
		typename std::enable_if<_IsJuxtaposed,bool>::type IsBackwardDup(skiplist_node_base* node);

		template<typename _Arg, typename _Node_gen, bool _IsJuxtaposed = IsJuxtaposed>
		typename std::enable_if<_IsJuxtaposed, iterator>::type
		_M_insert_(_Base_ptr node_info, _Arg&& val, _Node_gen node_gen);

		template<typename _Arg, typename _Node_gen, bool _IsJuxtaposed = IsJuxtaposed>
		typename std::enable_if<!_IsJuxtaposed, iterator>::type
		_M_insert_(_Base_ptr node_info, _Arg&& val, _Node_gen node_gen);


		template<bool _IsJuxtaposed = IsJuxtaposed>
		typename std::enable_if<_IsJuxtaposed>::type
		_M_erase_aux(const_iterator __pos) ;

		template<bool _IsJuxtaposed = IsJuxtaposed>
		typename std::enable_if<!_IsJuxtaposed>::type
		_M_erase_aux(const_iterator __pos) ;

		template<bool _IsJuxtaposed = IsJuxtaposed>
		typename std::enable_if<_IsJuxtaposed>::type
		_M_erase_aux(const_iterator __first, const_iterator __last) ;

		template<bool _IsJuxtaposed = IsJuxtaposed>
		typename std::enable_if<!_IsJuxtaposed>::type
		_M_erase_aux(const_iterator __first, const_iterator __last) ;
	public:
	  ~_skip_list() noexcept
	  { _M_erase_aux(_M_begin(),_M_end()); }
		
};

template<typename _Key, typename _Val, typename _KeyOfValue,typename _Compare, bool IsJuxtaposed, typename _Alloc>
template<typename _Arg>
std::pair<typename _skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::iterator, bool>
_skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::_M_insert_unique(_Arg&& arg)
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

template<typename _Key, typename _Val, typename _KeyOfValue,typename _Compare, bool IsJuxtaposed, typename _Alloc>
std::pair<typename _skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::_Base_ptr, typename _skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::_Base_ptr> 
_skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::_M_get_unique_pos(_Base_ptr node_info, const value_type& val)
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
				&&_M_impl._M_strict_compare(*static_cast<_Link_type>(node->level[i-1].forward)->val_ptr(),val))
		{
			span += node->level[i-1].span;
			node = static_cast<_Link_type>(node->level[i-1].forward);
		}
		node_info->level[i-1].forward = node;
	}

	if(node->level[0].forward==end||_M_impl._M_strict_compare(val,*static_cast<_Link_type>(node->level[0].forward)->val_ptr()))
	{
		return Res(nullptr,node_info);
	}else
	{
		return Res(node, node_info);
	}
}

template<typename _Key, typename _Val, typename _KeyOfValue,typename _Compare, bool IsJuxtaposed, typename _Alloc>
template<typename _Arg, typename _Node_gen, bool _IsJuxtaposed>
typename std::enable_if<!_IsJuxtaposed, typename _skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::iterator>::type
_skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::_M_insert_(_Base_ptr node_info, _Arg&& val,  _Node_gen node_gen)
{
	_Link_type __z = node_gen(std::forward<_Arg>(val));

	_Skip_list_insert_and_fix(__z,node_info,_M_impl);
	iterator iter(__z);

	_M_map.insert(typename map_type::value_type(_S_key(__z),iter));
	
	return iter;
}

template<typename _Key, typename _Val, typename _KeyOfValue,typename _Compare, bool IsJuxtaposed, typename _Alloc>
template<typename _Arg, typename _Node_gen, bool _IsJuxtaposed>
typename std::enable_if<_IsJuxtaposed, typename _skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::iterator>::type
_skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::_M_insert_(_Base_ptr node_info, _Arg&& val,  _Node_gen node_gen)
{
	_Link_type __z = node_gen(std::forward<_Arg>(val));

	bool IsForwardDups[skiplist_node_base::SKIPLIST_MAXLEVEL];
	skiplist_node_base *backward, *forward;
	for(skiplist_node_base::level_size i=_M_impl.level_cnt;i>0;--i)
	{
		backward = node_info->level[i-1].forward;
		forward = backward->level[i-1].forward;

		const value_type& forward_val = *static_cast<_Link_type>(forward)->val_ptr();
		IsForwardDups[i-1] = forward!=_M_impl.end()&&
					!_M_impl._M_compare(forward_val,val)&& !_M_impl._M_compare(val,forward_val);
	}
	const value_type& backward_val = *static_cast<_Link_type>(backward)->val_ptr();
	bool IsBackwardDup = backward != _M_end()&& 
					!_M_impl._M_compare(backward_val,val)&& !_M_impl._M_compare(val,backward_val);
	
	_Skip_list_insert_and_fix(__z,node_info, _M_impl, IsForwardDups, IsBackwardDup);

	iterator iter(__z);

	_M_map.insert(typename map_type::value_type(_S_key(__z),iter));
	
	return iter;
}

template<typename _Key, typename _Val, typename _KeyOfValue,typename _Compare, bool IsJuxtaposed, typename _Alloc>
template<bool _IsJuxtaposed>
typename std::enable_if<_IsJuxtaposed,bool>::type
_skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::IsForwardDup(skiplist_node_base* node, skiplist_node_base::level_size level)
{
	bool IsForwardDup =false;
	if(node->level[level].forward!=_M_impl.end())
	{
		const value_type& forward_val = *static_cast<_Link_type>(node->level[level].forward)->val_ptr();
		const value_type& val = *static_cast<_Link_type>(node)->val_ptr();
		IsForwardDup = !_M_impl._M_compare(forward_val,val)&& !_M_impl._M_compare(val,forward_val);
	}
	return IsForwardDup;
}

template<typename _Key, typename _Val, typename _KeyOfValue,typename _Compare, bool IsJuxtaposed, typename _Alloc>
template<bool _IsJuxtaposed>
typename std::enable_if<_IsJuxtaposed,bool>::type
_skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::IsBackwardDup(skiplist_node_base* node)
{
	bool IsBackwardDup =false;
	if(node->backward!=_M_header())
	{
		const value_type& backward_val = *static_cast<_Link_type>(node->backward)->val_ptr();
		const value_type& val = *static_cast<_Link_type>(node)->val_ptr();
		IsBackwardDup = !_M_impl._M_compare(backward_val,val)&& !_M_impl._M_compare(val,backward_val);
	}
	return IsBackwardDup;

}

template<typename _Key, typename _Val, typename _KeyOfValue,typename _Compare, bool IsJuxtaposed, typename _Alloc>
template<bool _IsJuxtaposed>
typename std::enable_if<!_IsJuxtaposed>::type
//template<typename std::enable_if<!IsJuxtaposed>::type>
//void
_skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::_M_erase_aux(const_iterator __pos)
{
	_Link_type __y = static_cast<_Link_type>(const_cast<_Base_ptr>(__pos.m_node));

	skiplist_node_base node_info = skiplist_node_base();
	auto res=_M_get_unique_pos(&node_info, *__y->val_ptr());
	if(res.first)
	{
		_M_map.erase(_M_map.find(_S_key(__y)));
		_Skip_list_erase(__y, &node_info, _M_impl);
		_M_drop_node(__y);
		--_M_impl.count;
	}
}

template<typename _Key, typename _Val, typename _KeyOfValue,typename _Compare, bool IsJuxtaposed, typename _Alloc>
template<bool _IsJuxtaposed>
typename std::enable_if<_IsJuxtaposed>::type
//template<typename std::enable_if<IsJuxtaposed>::type>
//void
_skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::_M_erase_aux(const_iterator __pos)
{
	_Link_type __y = static_cast<_Link_type>(const_cast<_Base_ptr>(__pos.m_node));

	skiplist_node_base node_info = skiplist_node_base();
	auto res=_M_get_unique_pos(&node_info, *__y->val_ptr());
	if(res.first)
	{
		skiplist_node_base* backward = node_info.level[0].forward;
		_M_map.erase(_M_map.find(_S_key(__y)));
		bool bIsDup = IsForwardDup(backward,0) || IsBackwardDup(backward);
		_Skip_list_erase(__y, &node_info, _M_impl,bIsDup);
		_M_drop_node(__y);
		--_M_impl.count;
	}
}

template<typename _Key, typename _Val, typename _KeyOfValue,typename _Compare, bool IsJuxtaposed, typename _Alloc>
template<bool _IsJuxtaposed>
typename std::enable_if<_IsJuxtaposed>::type
_skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::_M_erase_aux(const_iterator __first, const_iterator __last)
{
	_Link_type __y = static_cast<_Link_type>(const_cast<_Base_ptr>(__first.m_node));
	_Link_type __end = static_cast<_Link_type>(const_cast<_Base_ptr>(__first.m_node));

	skiplist_node_base node_info = skiplist_node_base();
	auto res=_M_get_unique_pos(&node_info, *__y->val_ptr());
	if(res.first)
	{
		skiplist_node_base* backward = node_info.level[0].forward;
		while(__y!=__end)
		{
			_M_map.erase(_M_map.find(_S_key(__y)));
			bool bIsDup = IsForwardDup(backward,0) || IsBackwardDup(backward);
			_Skip_list_erase(__y, &node_info, _M_impl, bIsDup);

			_Base_ptr next = __y->level[0].forward;
			_M_drop_node(__y);
			__y = static_cast<_Link_type>(next);
			--_M_impl.count;
		}
	}
}

template<typename _Key, typename _Val, typename _KeyOfValue,typename _Compare, bool IsJuxtaposed, typename _Alloc>
template<bool _IsJuxtaposed>
typename std::enable_if<!_IsJuxtaposed>::type
_skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::_M_erase_aux(const_iterator __first, const_iterator __last)
{
	_Link_type __y = static_cast<_Link_type>(const_cast<_Base_ptr>(__first.m_node));
	_Link_type __end = static_cast<_Link_type>(const_cast<_Base_ptr>(__first.m_node));

	skiplist_node_base node_info = skiplist_node_base();
	auto res=_M_get_unique_pos(&node_info, *__y->val_ptr());
	if(res.first)
	{
		while(__y!=__end)
		{
			_M_map.erase(_M_map.find(_S_key(__y)));
			_Skip_list_erase(__y, &node_info, _M_impl);

			_Base_ptr next = __y->level[0].forward;
			_M_drop_node(__y);
			__y = static_cast<_Link_type>(next);
			--_M_impl.count;
		}
	}
}

template<typename _Key, typename _Val, typename _KeyOfValue,typename _Compare, bool IsJuxtaposed, typename _Alloc>
template<typename _Up_Val, typename _ValOf>
_Up_Val& _skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::_select_key<_Up_Val,_ValOf>::at(const _Key& __k)
{
	return _Up_ValOf()(*_M_list._M_map.at(__k));
}

template<typename _Key, typename _Val, typename _KeyOfValue,typename _Compare, bool IsJuxtaposed, typename _Alloc>
template<typename _Up_Val, typename _ValOf>
const _Up_Val& _skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::_select_key<_Up_Val,_ValOf>::at(const _Key& __k) const
{
	const_iterator iter =_M_list._M_map.at(__k);
	return _Up_ValOf()(*iter);
}

template<typename _Key, typename _Val, typename _KeyOfValue,typename _Compare, bool IsJuxtaposed, typename _Alloc>
template<typename _Up_Val, typename _ValOf>
typename _skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::iterator _skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::_select_key<_Up_Val,_ValOf>::find(const _Key& __k)
{
	auto iter = _M_list._M_map.find(__k);
	if(iter==_M_list._M_map.end())
		return _M_list.end();
	else return iter->second;
}

template<typename _Key, typename _Val, typename _KeyOfValue,typename _Compare, bool IsJuxtaposed, typename _Alloc>
template<typename _Up_Val, typename _ValOf>
typename _skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::const_iterator _skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::_select_key<_Up_Val,_ValOf>::find(const _Key& __k) const
{
	auto iter = _M_list._M_map.find(__k);
	if(iter==_M_list._M_map.end())
		return const_iterator(_M_list._M_end());
	else return const_iterator(iter->second);
}

template<typename _Key, typename _Val, typename _KeyOfValue,typename _Compare, bool IsJuxtaposed, typename _Alloc>
template<typename _Up_Val, typename _ValOf>
template<bool _IsJuxtaposed>
typename std::enable_if<!_IsJuxtaposed,skiplist_node_header::size_type>::type
_skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::_select_key<_Up_Val, _ValOf>::rankofkey(const _Key& __k) const
{
	skiplist_node_base node_info= skiplist_node_base();
	auto iter = _M_list._M_map.find(__k);
	if(iter!=_M_list._M_map.end())
	{
		_M_list._M_get_unique_pos(&node_info,*iter->second);
		return node_info.level[0].span +
			node_info.level[0].forward->level[0].span;
	}else
	{
		throw std::out_of_range("key not found");
	}
}

template<typename _Key, typename _Val, typename _KeyOfValue,typename _Compare, bool IsJuxtaposed, typename _Alloc>
template<typename _Up_Val, typename _ValOf>
template<bool _IsJuxtaposed>
typename std::enable_if<_IsJuxtaposed,skiplist_node_header::size_type>::type
_skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::_select_key<_Up_Val, _ValOf>::rankofkey(const _Key& __k) const
{
	skiplist_node_base node_info= skiplist_node_base();
	auto iter = _M_list._M_map.find(__k);
	if(iter!=_M_list._M_map.end())
	{
		_M_list._M_get_unique_pos(&node_info,*iter->second);
		skiplist_node_base* backward = node_info.level[0].forward;
		return node_info.level[0].span +
			node_info.level[0].forward->level[0].span;
		//return node_info.level[0].span +(_M_list.IsForwardDup(backward,0)?0:1);
	}else
	{
		throw std::out_of_range("key not found");
	}
}

template<typename _Key, typename _Val, typename _KeyOfValue,typename _Compare, bool IsJuxtaposed, typename _Alloc>
_Val& _skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::_select_rank::at(size_type n)
{
	skiplist_node_base* node = _Skip_list_get_rank(_M_list._M_impl,n);
	if(node==_M_list._M_impl.end()) throw std::out_of_range("_skip_list rank out of range");
	else return *static_cast<_Link_type>(node)->val_ptr();
}

template<typename _Key, typename _Val, typename _KeyOfValue,typename _Compare, bool IsJuxtaposed, typename _Alloc>
const _Val& _skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::_select_rank::at(size_type n) const
{
	skiplist_node_base* node = _Skip_list_get_rank(const_cast<skiplist_node_header&>(_M_list._M_header),n);
	if(!node) throw std::out_of_range("_skip_list rank out of range");
	else {
		_Link_type link_node = static_cast<_Link_type>(node);
		return link_node->val_ptr();
	}
}

template<typename _Key, typename _Val, typename _KeyOfValue,typename _Compare, bool IsJuxtaposed, typename _Alloc>
skiplist_node_header::size_type _skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::_select_rank::count(size_type n)
{
	skiplist_node_base* node = _Skip_list_get_rank(_M_list._M_header,n);
	if(!node) return 0;
	else return 1;
}

template<typename _Key, typename _Val, typename _KeyOfValue,typename _Compare, bool IsJuxtaposed, typename _Alloc>
typename _skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::iterator _skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::_select_rank::find(size_type n)
{
	skiplist_node_base* node = _Skip_list_get_rank(_M_list._M_impl,n);
	//must consist with _skip_list.end()
	//if(!node) return _M_list.end();
	return iterator(node);
}

template<typename _Key, typename _Val, typename _KeyOfValue,typename _Compare, bool IsJuxtaposed, typename _Alloc>
typename _skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::const_iterator _skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::_select_rank::find(size_type n) const
{
	skiplist_node_base* node = _Skip_list_get_rank(const_cast<skiplist_node_header&>(_M_list._M_header),n);
	return iterator(node);
}

template<typename _Key, typename _Val, typename _KeyOfValue,typename _Compare, bool IsJuxtaposed, typename _Alloc>
_Key _skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::_select_rank::keyofrank(size_type n)
{
	skiplist_node_base* node = _Skip_list_get_rank(_M_list._M_impl,n);
	if(!node) throw std::out_of_range("_skip_list rank out of range");
	else {
		_Link_type link_node = static_cast<_Link_type>(node);
		return _KeyOfValue()(*link_node->val_ptr());
	}
}
template<typename _Key, typename _Val, typename _KeyOfValue,typename _Compare, bool IsJuxtaposed, typename _Alloc>
template<bool _IsJuxtaposed>
typename std::enable_if<_IsJuxtaposed,
						std::pair<
						typename _skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::iterator,
						typename _skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::iterator
						>
	>::type
_skip_list<_Key, _Val, _KeyOfValue, _Compare, IsJuxtaposed, _Alloc>::_select_rank::equal_range(size_type n)
{
	skiplist_node_base* first_node = _Skip_list_get_rank(_M_list._M_impl,n);
	skiplist_node_base* last_node = _Skip_list_get_rank(_M_list._M_impl,n+1);
	if(!last_node) return std::make_pair(iterator(first_node),_M_list.end());
	else return std::make_pair(iterator(first_node), iterator(last_node));

}
