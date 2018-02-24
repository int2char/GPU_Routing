/*************************************************
Copyright:UESTC
Author: ZhangQian
Date:2016-08-25
Description:cplex solve method header.
**************************************************/
#ifndef __Cplexh__
#define __Cplexh__
#include "Graph.h"
#include"service.h"
#include"taskPath.h"
#include<iostream>
#include<ostream>
#include <vector>
#include<map>
using namespace std;

class CplexM{

private:
	vector<float>demand;
	vector<float>capacity;
	taskPath* PathSets;
	vector<vector<pair<float,int>>>constraint;
	vector<pair<float,int> >object;
	vector<int>variables;
	map<int,int> va2d;
	map<int,int> va2p;
	Graph &Gm;
	vector<service>&serv;
public:
	//float static lowbound;
	CplexM(vector<service>&ser, taskPath*_PathSets, Graph &_G, ofstream&_Out);
	pair<int,vector<pair<string,float>>> solve();
	void inner();
	~CplexM(){};
};
#endif
