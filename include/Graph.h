#ifndef __GRAPH__

#define __GRAPH__

#include "Edge.h"
#include<vector>
using namespace std;
class Graph
{
public:
	~Graph();
	Graph(int _n, int _m,int GN);
	Graph(int _n, int _m, const char* file);
	Graph(int _n, int _m, const char* file,int );
	vector<Edge> edges;
	Edge*incL;
	vector<int>p;
	int n, m;
	float tw;
	vector<int> near[10000];
	vector<int> prear[10000];
	//vector<int> nredge[10000];
	void myway();
	void ER();
	void RG();
	void BA();
	//unordered_map<int, int> st2E;
	void FileInitial(const char* file);
	void FileDynamic(const char*file);
	void Outfile(char*file);
private:
	void Graph_addEdge(int s, int t, float w,float capacity, int &cnt);
	void addedge(int s, int t, float w, float c, int &cnt);
	void Graph_Initial(int GN);
};

#endif
