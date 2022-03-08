#include "skiplist.h"
#include <iostream>

template<typename pair>
struct KeyOfValue
{
	typename pair::first_type&
	operator()(pair &p)
	{
		return p.first;
	}
	const typename pair::first_type&
	operator()(const pair&p) const
	{
		return p.first;
	}

};

template<typename pair>
struct Com
{
	//typedef typename pair::first_type com_type;
	bool operator()(const pair& lhs, const pair& rhs)
	{
		return lhs.second < rhs.second;
	}
};

int main()
{
	using kov = KeyOfValue<std::pair<int,int>>;
	//using sov = ScoreOfValue<std::pair<int,int>>;
	using com = Com<std::pair<int,int>>;
	typedef skip_list<int,int> skiplist;
	skiplist list;
	list.insert(std::pair<int,int>(4,4));
	list.insert(std::pair<int,int>(2,2));
	list.insert(std::pair<int,int>(1,1));
	list.insert(std::pair<int,int>(3,3));
	std::cout<<(void *)(list.begin().m_node);
	auto res = list.insert(std::pair<int,int>(3,5));
	std::cout<<"insert res:"<<res.second<<"\n";
	res = list.insert(std::pair<int,int>(5,3));
	std::cout<<"insert res:"<<res.second<<"\n";
	for(skiplist::iterator iter = list.begin();iter!=list.end();iter++)
	{
		std::cout<<iter->first<<iter->second<<std::endl;
	}
}
