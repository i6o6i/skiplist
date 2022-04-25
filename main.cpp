#include "skiplist.h"
#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <string>
#include <cstddef>
#include <set>

template<bool IsJuxtapose>
using rank_container = std::vector<std::pair<int,int>>;

template<bool IsJuxtapose>
using rankofval_t = typename std::conditional<IsJuxtapose,
	  std::map<std::pair<int,int>,int,_compare2nd<std::pair<int,int>>>,
	  std::map<std::pair<int,int>,int>
	  >::type;

typedef std::vector<std::pair<int,int>> value_container_t ;
void test_case(const std::string& str, bool cond)
{
	if(cond)
		std::cout<<"test case: "<<str<<" success\n";
	else
		std::cout<<"test case: "<<str<<" failed\n";
}


template<bool IsJuxtapose>
void test_dup_rank(std::vector<std::pair<int,int>>& insert_data, skip_list<int,int,IsJuxtapose>& list);

template<>
void test_dup_rank<false>(std::vector<std::pair<int,int>>& insert_data, skip_list<int,int,false>& list)
{ }

template<>
void test_dup_rank<true>(std::vector<std::pair<int,int>>& insert_data, skip_list<int,int,true>& list)
{
	std::set<int> set_value;
	std::transform(insert_data.begin(),insert_data.end(),std::inserter(set_value,set_value.begin()),
				   [](const std::pair<int,int>& pair)
				   {
					   return pair.second;
				   }
	   );
	auto value_iter = set_value.begin();
	for(size_t i=1;i<=set_value.size();++i,++value_iter)
	{
		int val = *value_iter;
		std::vector<std::pair<int,int>> vec_dups_data;
		for(auto& pair:insert_data)
		{
			if(pair.second==val)
				vec_dups_data.push_back(pair);
		}

		auto pair_list_range = list.rank.equal_range(i);
		auto list_iter = pair_list_range.first;
		for(auto& pair:vec_dups_data)
		{
			if(list_iter!=list.end())
				test_case("equal_range "+std::to_string(i),pair.first == list_iter->first&&pair.second == list_iter->second);
			else test_case("equal_range "+std::to_string(i),false);
			++list_iter;
		}
	}
}

template<bool IsJuxtapose>
void test()
{
	typedef skip_list<int,int,IsJuxtapose> skiplist_t;
	typedef std::vector<std::pair<int,int>> test_container_t ;
	test_container_t insert_data = {
		{4,4},
		{2,2},
		{1,1},
		{3,3}
	};
	std::pair<int,int> inserted_data(5,3);
	value_container_t rankofkey_find_test_data;
	skiplist_t list;
	//insert test
	for(size_t i=0;i<insert_data.size();i++)
	{
		list.insert(insert_data[i]);
		rankofkey_find_test_data.push_back(insert_data[i]);
	}
	auto res = list.insert(std::pair<int,int>(3,5));
	test_case("dup key insert test:", res.second == false);
	res = list.insert(inserted_data);
	test_case("dup value insert test:", res.second == true);

	insert_data.push_back(std::pair<int,int>(5,3));
	rankofkey_find_test_data.push_back(std::pair<int,int>(5,3));

	std::sort(insert_data.begin(),insert_data.end(),[](const std::pair<int,int>& p1,const std::pair<int,int>& p2){
		return p1.second < p2.second || (p1.second == p2.second && std::memcmp(&p1,&p2, sizeof(std::pair<int,int>))<0);
	});
	std::sort(rankofkey_find_test_data.begin(),rankofkey_find_test_data.end(),[](const std::pair<int,int>& p1,const std::pair<int,int>& p2){
		return p1.second < p2.second || (p1.second == p2.second && std::memcmp(&p1,&p2, sizeof(std::pair<int,int>))<0);
	});

	if(IsJuxtapose)
	{
		auto iter = rankofkey_find_test_data.begin();
		while(iter!=rankofkey_find_test_data.end())
		{
			auto iter2 =iter;
			iter2++;
			while(iter2!=rankofkey_find_test_data.end())	
			{
				if(iter2->second==iter->second)
				{
					iter2 = rankofkey_find_test_data.erase(iter2);
				}
				else
				{
					++iter2;
				}
			}
			iter++;
		}
	}
	rankofval_t<IsJuxtapose> rankofval;
	int rank=1;
	std::transform(rankofkey_find_test_data.begin(),
				   rankofkey_find_test_data.end(),
				   std::inserter(rankofval,rankofval.begin()),
				   [&rank](std::pair<int,int> pair){
					   return std::make_pair(pair,rank++);
				   });

	typename skiplist_t::iterator list_iter = list.begin();
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

	//find test
	size_t i=1;
	for(auto iter=insert_data.begin();iter!=insert_data.end();iter++,i++)
	{
		std::string case_name="skiplist.key test: ("+std::to_string(iter->first)+","+std::to_string(iter->second)+") :";
		auto key_helper_iter = list.key.find(iter->first);
		test_case(case_name, list.key.rankofkey(iter->first)==rankofval.at(*iter)&&list.key.at(iter->first)==iter->second
				  &&key_helper_iter->first == iter->first && key_helper_iter->second ==iter->second
				  );
	}
	auto test_data_iter = rankofkey_find_test_data.begin();
	for(i=1;
		i<=rankofkey_find_test_data.size()&&test_data_iter!=rankofkey_find_test_data.end();
		++i,++test_data_iter)
	{
		std::string case_name="skiplist.rank test: rank "+std::to_string(i)+") :";

		auto rank_helper =list.rank;
		auto rank_helper_iter = rank_helper.find(i);
		test_case(case_name, 
				  //rank_helper.keyofrank(i)==test_data_iter->first
				  //&&
				  rank_helper.at(i).second==test_data_iter->second
				  //&&rank_helper_iter->first == test_data_iter->first
				  && rank_helper_iter->second ==test_data_iter->second
				  );
	}
	//Juxtapose test
	test_dup_rank<IsJuxtapose>(rankofkey_find_test_data,list);
	//erase test
	list_iter = list.begin();
	test_case("test size ",list.size()== insert_data.size());
	while(list_iter!=list.end())
	{
		list_iter = list.erase(list_iter);
	}

	test_case("test size ",list.size() == 0);
	test_case("test empty ",list.empty() == true);
	test_case("erase all test",list.begin()==list.end());
}

int main()
{
	enum {NotJuxtapose=0, Juxtapose=1};
	test<NotJuxtapose>();
	test<Juxtapose>();
}
