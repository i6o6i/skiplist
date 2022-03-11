#include "skiplist.h"
#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <string>

void test_case(const std::string& str, bool cond)
{
	if(cond)
		std::cout<<"test case: "<<str<<" success\n";
	else
		std::cout<<"test case: "<<str<<" failed\n";
}
int main()
{
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
	auto res = list.insert(std::pair<int,int>(3,5));
	test_case("dup key insert test:", res.second == false);
	res = list.insert(std::pair<int,int>(5,3));
	test_case("dup value insert test:", res.second == true);

	insert_data.push_back(std::pair<int,int>(5,3));

	std::sort(insert_data.begin(),insert_data.end(),[](const std::pair<int,int>& p1,const std::pair<int,int>& p2){
		return p1.second < p2.second || (p1.second == p2.second && p1.first < p2.first);
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

	//find test
	size_t i=1;
	for(auto iter=insert_data.begin();iter!=insert_data.end();iter++,i++)
	{
		std::string case_name="skiplist.key test: ("+std::to_string(iter->first)+","+std::to_string(iter->second)+") :";
		auto key_helper =list.key;
		auto key_helper_iter = key_helper.find(iter->first);
		test_case(case_name, key_helper.rankofkey(iter->first)==i&&key_helper.at(iter->first)==iter->second
				  &&key_helper_iter->first == iter->first && key_helper_iter->second ==iter->second
				  );

		case_name="skiplist.rank test: ("+std::to_string(iter->first)+","+std::to_string(iter->second)+") :";

		auto rank_helper =list.rank;
		auto rank_helper_iter = rank_helper.find(i);
		test_case(case_name, rank_helper.keyofrank(i)==iter->first&&rank_helper.at(i).second==iter->second
				  &&rank_helper_iter->first == iter->first && rank_helper_iter->second ==iter->second
				  );
	}
	//erase test
	skiplist::iterator iter = list.begin();
	test_case("test size ",list.size()== insert_data.size());
	while(iter!=list.end())
	{
		iter = list.erase(iter);
	}

	test_case("test size ",list.size() == 0);
	test_case("test empty ",list.empty() == true);
	test_case("erase all test",list.begin()==list.end());

}
