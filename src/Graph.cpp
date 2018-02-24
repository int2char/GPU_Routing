#include "Graph.h"
#include <iostream> 
#include<fstream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iostream>
#include<map>
#include<math.h>
#include"const.h"
#define INF 1000000
#define COSTINF 7
#define CAPACITYINF 9000000
# define PI  3.14159265358979323846 
/*void Graph::myway(){
	printf("init graph...\n");
	map<pair<int, int>, int>flag;
	srand((unsigned)time(NULL));
	int count = 0;
	int cc = 0;
	for (int i = 1; i < this->n; i++)
	{
		float c = 100;
		float w = 10.0;
		Graph_addEdge(i, i - 1, w, c, count);
		flag.insert(make_pair(make_pair(i, i - 1), 1));
		flag.insert(make_pair(make_pair(i - 1, i), 1));
		cc = cc + 2;
	}
	this->m = count;
	printf("graph inint success! edge num:%d\n", m);
}
void Graph::ER(){
	int count = 0;
	map<pair<int, int>, int>flag;
	double threhod = log((double)(n));
	for (int i = 0; i <this->n; i++)
		for (int j = 0; j < this->n; j++)
			if (i != j)
			{
				double ran =(double) (rand() % (this->n));
				if (ran<threhod)
				{
					if (flag.find(make_pair(i, j)) == flag.end())
					{
						Graph_addEdge(i, j, 10, 100, count);
						flag.insert(make_pair(make_pair(i,j), 1));
						flag.insert(make_pair(make_pair(j,i), 1));

					}
				}
			}
	this->m = count;
}
void Graph::RG(){
	double*x = new double[n];
	double*y = new double[n];
	map<pair<int, int>, int>flag;
	int count = 0;
	for (int i = 0; i < n; i++)
	{
		int f1 = rand();
		
		x[i]= (double)f1/(double)RAND_MAX;		
	}
	for (int i = 0; i < n; i++)
	{
		int f2 = rand();

		y[i] = (double)f2 / (double)RAND_MAX;
	}
	double diste = sqrt((6.0 +(6-log10(n))/10.0)/(PI*(double)n));// sqrt(log10(20*n) / (PI*n));
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
		{
			if (i != j)
			{
				if (sqrt(pow((x[i] - x[j]), 2) + pow((y[i] - y[j]), 2)) < diste)
					if (flag.find(make_pair(i, j)) == flag.end())
					{
						Graph_addEdge(i, j, 10, 100, count);
						flag.insert(make_pair(make_pair(i, j), 1));
						flag.insert(make_pair(make_pair(j, i), 1));
					}
			}
		}
	this->m = count;
	
}
void Graph::BA(){
	int todu = 0;
	int count = 0;
	int k =3;
	int *du = (int*)calloc(n,sizeof(int));
	for (int i = 0; i < 5; i++)
	{
		Graph_addEdge(i,i+1,10,100,count);
		du[i]++;
		du[i + 1]++;
		todu += 2;
	}
	for (int i = 5; i < n; i++)
	{
		int addin = 0;
		while (addin < k)
		{
			for (int j = 0; j < n; j++)
			{
				if (rand() % todu < du[j])
				{
					Graph_addEdge(i,j,10,100,count);
					du[i]++;
					du[j]++;
					todu += 2;
					addin++;
				}
				if (addin >= k)
					break;
			}
		}
	}
	this->m = count;
}
void Graph::Graph_Initial(int GN) {
	if (GN==0)
	    ER();
	if (GN==1)
	    RG();
	if (GN==2)
		BA();
}*/
Graph::Graph(int _n, int _m,const char* file) {
	this->n = _n;
	this->tw =0;
	this->FileInitial(file);

}
Graph::Graph(int _n, int _m, const char* file,int) {
	this->n = _n;
	this->tw = 0;
	this->FileDynamic(file);

}
Graph::~Graph()
{
	cout << "here" ;
}
bool cmp(Edge a, Edge b)
{
	return a.capacity<b.capacity;
}
void Graph::addedge(int s, int t, float w, float c, int &cnt){
	Edge e(s,t,w,c,w);
	this->edges.push_back(e);
	this->near[s].push_back(cnt);
	this->prear[t].push_back(s);
	cnt++;

}
void Graph::FileInitial(const char* file){
	FILE*rgrap = fopen(file, "r");
	int s=0, t=0;
	float c=0;
	int cnt = 0;
	while (fscanf(rgrap,"%d %d %f", &s, &t, &c) == 3)
		{
			addedge(s,t,1.0,CAPACITY,cnt);
			addedge(t,s,1.0,CAPACITY,cnt);
		}
	this->m = cnt;
	this->incL=(Edge*)malloc(sizeof(Edge)*m);
	for(int i=0;i<this->m;i++)
		incL[i]=edges[i];

}
void Graph::FileDynamic(const char*file){
	FILE*rgrap = fopen(file, "r");
	int s = 0, t = 0;
	float c = 0;
	int cnt = 0;
	while (fscanf(rgrap, "%d %d %f", &s, &t, &c) == 3)
		addedge(t,s, 1.0, c, cnt);
	this->m = cnt;
	this->incL=(Edge*)malloc(sizeof(Edge)*m);
	for(int i=0;i<this->m;i++)
		incL[i]=edges[i];
}
void Graph::Outfile(char*file){
}
