#ifndef __GRAPHPATH__
#define __GRAPHPATH__

#include "Graph.h"
#include"service.h"
#include"taskPath.h"
#include<iostream>
#include<ostream>
#include <vector>
#include <thrust/device_vector.h>
using namespace std;
class GraphPath
{
public:
	vector<pair<string,float> >bellmanFordCuda(vector<service>&ser,ostream& Out);
	GraphPath(Graph &_G);
	~GraphPath();
private:
	Graph G;
	Edge *dev_edge;
	const int numThread = 1024;
	void Copy2GPU(std::vector<service> &s);
	void CudaFree();
	float *dev_d = 0;
	int *dev_p = 0;
	int *dev_st;
	int *dev_te;
	float *dev_pd;
	float *dev_lambda = 0;
	int *dev_m;
	int *dev_mask;
	int cudasize;
	int devicesize;
	int *st;
	int *te;
	float *pd;
	float *d;
	int *pre;
	float *lambda;
	int* mask;
	int*mark;
	int stillS;
	vector<vector<int> > StoreRoute;
	vector<vector<int> > BestRoute;
	float*capacity;
};

#endif
