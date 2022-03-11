#include "skiplist.h"
#include <cstdlib>
#include <iostream>

constexpr float Probability = 0.5;
typedef typename skiplist_node_base::level_size level_size;
static level_size randomlevel()
{
	level_size level = 1;
	while ((rand()&0xFFFF) < (Probability * 0xFFFF))
		level += 1;
	return (level<skiplist_node_base::SKIPLIST_MAXLEVEL) ? level : skiplist_node_base::SKIPLIST_MAXLEVEL;
}

skiplist_node_base* _Skip_list_increment(const skiplist_node_base* node) throw ()
{
	//std::cout<<(void*)node->level[0].forward;
	return node->level[0].forward;
}

skiplist_node_base* _Skip_list_decrement(const skiplist_node_base* node) throw ()
{
	return node->backward;
}

template<bool IsJuxtaposed>
static void _Skip_list_insert_and_fix_internal(const skiplist_node_header& header, skiplist_node_base* __x,
											   skiplist_node_base* node_info);

template<>
static void _Skip_list_insert_and_fix_internal<true>(const skiplist_node_header& header, skiplist_node_base* __x,
												 skiplist_node_base* __x,
											   skiplist_node_base* node_info)
{
	bool IsForwardDups[skiplist_node_base::SKIPLIST_MAXLEVEL];
	for(skiplist_node_base::level_size i=0;i<level;++i)
	{
		skiplist_node_base* backward = node_info->level[i].forward;
		IsForwardDups[i] = 
			!(backward->level[i].forward!=header.end()&&
			  !static_cast<_Link_type>(backward->level[i].forward)->)
	}
	bool IsBackwardDup = node_info->level[i].forward && ==;
	for(level_size i=0;i<level;i++)
	{
		skiplist_node_base* backward_node = node_info->level[i].forward;
		__x->level[i].forward = backward_node->level[i].forward;
		backward_node->level[i].forward = __x; 
		skiplist_node_base::span_type dif = node_info->level[0].span - node_info->level[i].span;
		__x->level[i].span = IsForwardDups[i]?0:backward_node->level[i].span-dif - (IsForwardDups[0]?1:0);
		backward_node->level[i].span = dif+ (IsBackwardDup? 0:1);
	}


	for(level_size i=level; i< __header.level_cnt; i++)
	{
		if(!(IsBackwardDup||IsForwardDups[0]))
			node_info->level[i].forward->level[i].span++;
	}

	if(!(IsBackwardDup||IsForwardDups[0]))
		_M_impl.count++;

}

template<>
static void _Skip_list_insert_and_fix_internal<false>(const skiplist_node_header& header, skiplist_node_base* __x,
												 skiplist_node_base* __x,
											   skiplist_node_base* node_info)
{

	for(level_size i=0;i<level;i++)
	{
		skiplist_node_base* backward_node = node_info->level[i].forward;
		__x->level[i].forward = backward_node->level[i].forward;
		backward_node->level[i].forward = __x; 
		skiplist_node_base::span_type dif = node_info->level[0].span - node_info->level[i].span;
		__x->level[i].span = backward_node->level[i].span-dif;
		backward_node->level[i].span = dif+ 1;
	}

	for(level_size i=level; i< __header.level_cnt; i++)
	{
		node_info->level[i].forward->level[i].span++;
	}

	_M_impl.count++;
}

template<bool IsJuxtaposed>
void _Skip_list_insert_and_fix(skiplist_node_base* __x,
		skiplist_node_base* node_info, 
		skiplist_node_header& __header) throw ()
{
	level_size level = randomlevel();

	if(level > __header.level_cnt)
	{
		for(level_size i=__header.level_cnt;i<level;i++)
		{
			__header._M_header.level[i].forward = &__header._M_header;
			__header._M_header.level[i].span = __header.count;
			node_info->level[i].forward = &__header._M_header;
		}
		__header.level_cnt = level;
	}

	_Skip_list_insert_and_fix_internal<IsJuxtaposed>(__x,node_info);

	__x->backward = node_info->level[0].forward;
	if(__x->level[0].forward)
	{
		__x->level[0].forward->backward = __x;
	}
	if(__x->backward == &__header._M_header)
		__header._M_header.backward = __x;
	
}

void _Skip_list_erase(skiplist_node_base* __x,
		skiplist_node_base* node_info, 
		skiplist_node_header& __header) throw ()
{
		if(backward_node->level[i].forward == __x)
		{
			if(!( forwarddup ||backwarddup))
				backward_node->level[i].span += __x->level[i].span - 1;
			backward_node->level[i].forward = __x->level[i].forward;
		}else
		{
			if(!( forwarddup ||backwarddup))
				backward_node->level[i].span -= 1;
		}

}
void _Skip_list_erase(skiplist_node_base* __x,
		skiplist_node_base* node_info, 
		skiplist_node_header& __header) throw ()
{
	for(level_size i=0;i< __header.level_cnt;i++)
	{
		skiplist_node_base* backward_node = node_info->level[i].forward;
		if(backward_node->level[i].forward == __x)
		{
			backward_node->level[i].span += __x->level[i].span - 1;
			backward_node->level[i].forward = __x->level[i].forward;
		}else
		{
			backward_node->level[i].span -= 1;
		}
	}

	if(__header._M_header.backward == __x)
	{
		__header._M_header.backward = __x->level[0].forward;
	}
	if(__x->level[0].forward != &__header._M_header)
	{
		__x->level[0].forward->backward = __x->backward;
	}
	while(__header.level_cnt > 1 && __header._M_header.level[0].forward==&__header._M_header)
		__header.level_cnt--;
}

skiplist_node_base* _Skip_list_get_rank(skiplist_node_header& header, skiplist_node_header::size_type rank)
{
	skiplist_node_header::size_type span_sum =0;
	skiplist_node_base* node = &header._M_header;
	//skiplist_node_base* end = header._M_header;
	for(skiplist_node_base::level_size i= header.level_cnt;i>0&&span_sum<rank;i--)
	{
		//assum end is header._M_header
		while(node->level[i-1].forward!=&header._M_header&& span_sum + node->level[i-1].span <= rank)
		{
			span_sum += node->level[i-1].span;
			node =node->level[i-1].forward;
		}
		if(span_sum == rank)
			return node;
	}
	return nullptr;
}

