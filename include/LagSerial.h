/*************************************************
Copyright:UESTC
Author: ZhangQian
Date:2016-08-25
Description:LR-SROA method header
**************************************************/
#ifndef __LagSerial__
#define __LagSerial__
#include "Graph.h"
#include"service.h"
#include"taskPath.h"
#include<iostream>
#include<ostream>
#include <vector>
using namespace std;
class LagSerial
{
public:
	vector<pair<string,float> > dijkstraSerial(vector<service> &s,ostream&Out = cout);
	LagSerial(Graph &_G);
	~LagSerial();
private:
	Graph G;
	int devicesize;
	int *st;/*sevices start node array*/
	int *te;/*destination node array*/
	float *pd;/*demand bandwidth array*/
	float *d;/*distance array*/
	int *pre;/*precursor node array*/
	float *lambda;/*penalty weight*/
	int* mask;/*store the services ID need to be rearranged.*/
	vector<vector<int>>StoreRoute;/*routes cache*/
	vector<vector<int>>BestRoute;/*best routes ever found*/
	float*capacity;/*link capacity*/
};

#endif
