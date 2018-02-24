#pragma once
#define Kb 10
#include<vector>
using namespace std;
class taskPath
{
public:
	int num;
	vector<int> Pathset[Kb + 5];
public:
	taskPath();
	~taskPath();
};

