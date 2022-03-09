#include "skiplist.h"
#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <string>

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

void test_case(const std::string& str, bool cond)
{
	if(cond)
		std::cout<<"test case: "<<str<<" success\n";
	else
		std::cout<<"test case: "<<str<<" failed\n";
}
int main()
{
	using kov = KeyOfValue<std::pair<int,int>>;
	//using sov = ScoreOfValue<std::pair<int,int>>;
	using com = Com<std::pair<int,int>>;
	typedef skip_list<int,int> skiplist;
	std::vector<std::pair<int,int>> insert_data = {
		{4,4},
		{2,2},
		{1,1},
		{3,3}
	};
	skiplist list;
	//insert test
	for(size_t i=0;i<insert_data.size();i++)
	{
		list.insert(insert_data[i]);
	}

	std::sort(insert_data.begin(),insert_data.end(),[](const std::pair<int,int>& p1,const std::pair<int,int>& p2){
		return p1.second < p2.second;
	});
	skiplist::iterator list_iter = list.begin();
	auto test_case_iter = insert_data.begin();
	for(;list_iter!=list.end()&&test_case_iter!=insert_data.end();list_iter++,test_case_iter++)
	{
		std::string case_name="insert test: ("+std::to_string(test_case_iter->first)+","+std::to_string(test_case_iter->second)+")";
		test_case(case_name,
				  list_iter->first==test_case_iter->first
				  &&list_iter->second == test_case_iter->second
				  );
	}
	test_case("insert intergration test:", list_iter==list.end()&&test_case_iter==insert_data.end());

	auto res = list.insert(std::pair<int,int>(3,5));
	test_case("dup key insert test:", res.second == false);
	res = list.insert(std::pair<int,int>(5,3));
	test_case("dup value insert test:", res.second == false);
	//erase test
	skiplist::iterator iter = list.begin();
	while(iter!=list.end())
	{
		iter = list.erase(iter);
	}

	test_case("erase all test",list.begin()==list.end());
}
