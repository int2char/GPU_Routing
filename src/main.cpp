/*************************************************
Copyright:UESTC
Author: zhangqian
Date:2010-08-25
Description:Network data preprocessing fuctions,
and LR-PROA,LR-SROA,GA-PROA,GA-SPOA entrance functions
**************************************************/
#include "Graph.h"
#include"service.h"
#include "GraphPath.h"
#include"LagSerial.h"
#include"Compare.h"
#include"NewGA.h"
#include <time.h>
#include <iostream> 
#include <algorithm>
#include <vector>
#include<fstream>
#include"taskPath.h"
#include "const.h"
#include"CplexM.h"
#include"GAparrel.h"
#include"time.h"
#include<map>
#include <sys/time.h>
struct snode {
	service ser;
	int mark;
	snode(int s, int t, float d, int m) :ser(s, t, d), mark(m){}
};
float totaldemand = 0;
bool snUper(snode a, snode b){
	if (a.ser.d >b.ser.d)
		return true;
	return false;
}
Graph inline GenerateGraph(int n, int m){
	Graph G(n, m,1);
	return G;
}
void inline	WriteService(vector<service>&ser)
{
	ofstream outfile("service.txt");
	for (int i = 0; i < Task; i++)
	 outfile << ser[i].s << " " << ser[i].t << " " << ser[i].d<< endl;
	outfile.close();
}
void inline	WriteGraph(Graph &G,char*file)
{
	ofstream outfile(file);
	for (int i = 0; i < G.m; i=i+2)
	{   
		outfile << G.incL[i].tail << " " << G.incL[i].head << " " << G.incL[i].capacity << endl;
	}
	outfile.close();
}
void inline WriteFile(Graph &G,vector<service>&ser){
	WriteService(ser);
	WriteGraph(G, "Graph.txt");
	ofstream outfile("data.txt");
	int cnt = 0;
	for (int i =0; i < Task; i++)
	{
		vector<vector<int>> vec;
		for (int j = 0; j < vec.size(); j++)
		{   
			for (int k = 0; k < vec[j].size(); k++)
			{
				outfile <<","<<vec[j][k];
				cnt++;
			}
			outfile<<"|";
		}
		outfile << endl;
		vec.clear();
	}
	cout <<"total size:"<<cnt << endl;
	outfile.close();
}
void inline ReadService(vector<service>&ser, const char*file)
{
	FILE* rser = fopen(file, "r");
	int s, t;
	float d;
	int shit = Task+FROM;
	int iter = 0;
	while ((fscanf(rser, "%d %d %f", &s, &t, &d) == 3)&&shit--)
	{
		
		if (iter>=FROM)
		   ser.push_back(service(s, t, d));
		iter++;
	}
	fclose(rser);
}
bool downer(int a, int b)
{
	if (a > b)
		return true;
	return false;
}
void inline ChangDmand(vector<service>&ser){
	srand(1);
	vector<int> der;
	for (int i = 0; i < Task; i++)
		der.push_back(1 + rand() % 100);
	sort(der.begin(), der.end(), downer);
	for (int i = 0; i < Task; i++)
		ser[i].d = (float)der[i];
}
Graph inline ReadFile(int n,int m,vector<service>&ser){
	string s1=INPUTFILE;
	string sf=FLOWFILE;
	string st=TOPFILE;
	sf=s1+sf;
	st=s1+st;
	cout<<"ssadsa"<<sf;
	ReadService(ser,sf.c_str());
	if(GRAPHTYPE>0)
		return Graph(n,m,st.c_str());
	else
		return Graph(n,m,st.c_str(),1);

}
int split(char dst[][10000], char* str, const char* spl)
{
	int n = 0;
	char *result = NULL;
	result = strtok(str, spl);
	while (result != NULL)
	{
		strcpy(dst[n++], result);
		result = strtok(NULL, spl);
	}
	return n-1;
}
char* readline(FILE* f)
{
	char* line = (char*)calloc(1000, sizeof(char));;
	char c;
	int len = 0;

	while ((c = fgetc(f)) != EOF && c != '\n')
	{
		line[len++] = c;
		line[len] = '/0';
	}

	return line;
}
void inline GetPath(Graph& G, taskPath*Path){
	string s1=INPUTFILE;
	string s2=s1+string(ROUTFILE);
	FILE* data = fopen(s2.c_str(),"r");
	int tnum = 0;
	int shit = Task + FROM;
	char line[100000];
	int iter = 0;
	while (shit--){
		fgets(line, 99999, data);
		if (line[0] == '\0')
			break;
		char dst[20][10000];
		int n = split(dst, line, "|");
		int aci = 0;
		if (iter >= FROM)
		{
			for (int i = 0; i < n; i++)
			{
				char*dem = dst[i];
				int elem = 0;
				int k = 0;
			std:map<int, int> remap;
				int flag = 0;
				while (sscanf(dem, ",%d", &elem) == 1)
				{
					//cout << G.incL[elem].head << "=>";
					if (remap.find(G.incL[elem].head) != remap.end())
					{
						cout << tnum << " " << i << " erro" << endl;
						//flag = 1;
					}
					remap.insert(make_pair(G.incL[elem].head, 1));
					Path[tnum].Pathset[i].push_back(elem);
					//cout << tnum <<" "<< i<<" " << k++<<" "<< endl;
					dem = dem + 2;
					if (elem > 9)
						dem++;
					if (elem > 99)
						dem++;
					if (elem > 999)
						dem++;
					if (elem > 9999)
						dem++;
					if (elem > 99999)
						dem++;
					if (elem > 999999)
						dem++;
				}
				Path[tnum].Pathset[i].push_back(-1);
				if(Path[tnum].Pathset[i].size()-1>=INFHOPS)
				{
					Path[tnum].Pathset[i].clear();
					n=i;
				}
				remap.clear();
			}
			Path[tnum].num = n;
			tnum++;
		}
		iter++;
	}
	fclose(data);
}
void GuService(Graph&G,vector<service>&ser,vector<snode>&ser2)
{

	taskPath*Path = new taskPath[Task];
	GetPath(G, Path);
	float total = 0;
	int addsize = 0;
	for (int i = 0; i < Task; i++)
	{
		float min = 100;
		for (int j = 0; j < Path[i].num; j++)
			for (int k = 0; Path[i].Pathset[j][k]>0; k++)
				if (min > G.incL[Path[i].Pathset[j][k]].capacity)
					min = G.incL[Path[i].Pathset[j][k]].capacity;
		int realflow;
		if (min > 0)
		{
			realflow= rand() % (int)(min)+1;
			total += realflow;
			addsize++;
			ser2.push_back(snode(ser[i].s, ser[i].t, realflow, i));

		}
	
	}
	sort(ser2.begin(), ser2.end(), snUper);
	cout << "total is" << total << endl;
	cout << "total size is:" << addsize << endl;
	ofstream outd("data.txt");
	ofstream outs("service.txt");
	for (int i = 0; i < ser2.size(); i++)
	{
		outs << ser2[i].ser.s << " " << ser2[i].ser.t << " " << ser2[i].ser.d << endl;
		int routenum = Path[ser2[i].mark].num;
		for (int j=0; j < routenum; j++)
		{
			
			for (int k = 0; Path[ser2[i].mark].Pathset[j][k] >=0; k++)
				outd << "," << Path[ser2[i].mark].Pathset[j][k];
			outd << "|";
		}
		outd << endl;
	}
	outd.close();
	outs.close();
}
/*************************************************
Function:       Pathcheck
Description:    check services and paths read from file
*************************************************/
void inline Pathcheck(Graph &G, vector<service>&ser){
	int less=0;
	taskPath*Path = new taskPath[Task];
	string s1=INPUTFILE;
	string s2=s1+string(ROUTFILE);
	FILE* data = fopen(s2.c_str(),"r");
	int tnum = 0;
	int shit = Task + FROM;
	char line[100000];
	int iter = 0;
	while (shit--){
		fgets(line, 99999, data);
		if (line[0] == '\0')
			break;
		char dst[20][10000];
		int n = split(dst, line, "|");
		int aci = 0;
		if (iter >= FROM)
		{
			for (int i = 0; i < n; i++)
			{
				char*dem = dst[i];
				int elem = 0;
				int k = 0;
			    std:map<int, int> remap;
				int flag = 0;
				int first=0;
				while (sscanf(dem, ",%d", &elem) == 1)
				{
					if(first==0){
						remap.insert(make_pair(G.incL[elem].tail, 1));
						first=1;

					}
					if (remap.find(G.incL[elem].head) != remap.end())
					{
						cout << tnum << " " << i << " erro" << endl;
					}
					remap.insert(make_pair(G.incL[elem].head, 1));
					Path[tnum].Pathset[i].push_back(elem);
					dem = dem + 2;
					if (elem > 9)
						dem++;
					if (elem > 99)
						dem++;
					if (elem > 999)
						dem++;
					if (elem > 9999)
						dem++;
					if (elem > 99999)
						dem++;
					if (elem > 999999)
						dem++;
				}
				Path[tnum].Pathset[i].push_back(-1);
				if(Path[tnum].Pathset[i].size()-1>=INFHOPS)
				{
					Path[tnum].Pathset[i].clear();
					n=i;
				}
				remap.clear();
			}
			Path[tnum].num = n;
			tnum++;
		}
		iter++;
	}
	for(int i=0;i<Task;i++)
			{if (Path[i].num<10)
				less++;
			for(int j=0;j<Path[i].num;j++)
			{
				if(ser[i].s!=G.incL[Path[i].Pathset[j][0]].tail)
					cout<<"wrong tail";
				int k=0;
				while(true)
				{
					if(Path[i].Pathset[j][k]<0)
						break;
					if(Path[i].Pathset[j][k+1]>=0)
						if(G.incL[Path[i].Pathset[j][k]].head!=G.incL[Path[i].Pathset[j][k+1]].tail)
							cout<<"unconected"<<endl;
					k++;
				}
				if(ser[i].t!=G.incL[Path[i].Pathset[j][k-1]].head)
					cout<<"head wrong "<<endl;
			}
			}
	delete[]Path;
	fclose(data);
}

/*************************************************
Function:       Lag_Parrel
Description:    LR-PROA method
*************************************************/
void inline Lag_Parrel(Graph &G, vector<service>&ser,ofstream&outfile)
{	GraphPath OP(G);
	OP.bellmanFordCuda(ser,outfile);
}
/*************************************************
Function:       Lag_Serial
Description:    LR-SROA method
*************************************************/
void inline Lag_Serial(Graph &G, vector<service>&ser,ofstream&outfile){
	LagSerial ls(G);
	ls.dijkstraSerial(ser,outfile);
}
/*************************************************
Function:       GA_Parrel
Description:    GA-PROA method
*************************************************/
void inline GA_Parrel(Graph &G, vector<service>&ser,ofstream&outfile)
{

	int edge =G.m;
	taskPath*Path = new taskPath[Task];
	GetPath(G,Path);
	NewGAParrel Gs(ser,Path,G,outfile);
	Gs.GAsearch();
	delete[]Path;
}
/*************************************************
Function:       GA_Serial
Description:    GA-SROA method
*************************************************/
void inline GA_Serial(Graph &G, vector<service>&ser,ofstream&outfile)
{
	int edge =G.m;
	taskPath*Path = new taskPath[Task];
	GetPath(G,Path);
	NewGA Gs(G);
	Gs.GAsearch(ser,Path,outfile);
	delete[]Path;
}
/*************************************************
Function:       Cplexsolve
Description:    solve problem by cplex
*************************************************/
void inline Cplexsolve(Graph &G, vector<service>&ser,ofstream&outfile)
{
	int edge =G.m;
	taskPath*Path = new taskPath[Task];
	GetPath(G,Path);
	long long L1,L2;
	timeval tv;
	gettimeofday(&tv,NULL);
	L1=(tv.tv_sec*1000*1000 + tv.tv_usec);

	CplexM cplex(ser, Path, G, outfile);
	pair<int,vector<pair<string,float>>> redata=cplex.solve();
	gettimeofday(&tv,NULL);
	L2=(tv.tv_sec*1000*1000 + tv.tv_usec);
	redata.second.push_back(make_pair(string("time"),float((L2-L1)/1000)));
	redata.second.push_back(make_pair(string("iter_time"),float(L2-L1)/(1000*redata.first)));
	writejsondata(DATAFILE,redata.second,string("Cplex_solve"));

}
/*************************************************
Function:main
some paramter defined when compling
NODE:network node number.
Task:service number.
EDge:network edge number.
GRAPHTYPE:dynamic Graph(some links has been occupied) or static graph(no links has been occupied);value:dynamic=-1,static=1.
TYPE:BA or ER Grpah;BA=2,ER=0.
*************************************************/
int main(int args,char*arg[])
{
	int n = NODE;
	int m = EDge;
	std::vector<service> ser;
	Graph G=ReadFile(n,m,ser);
	totaldemand = 0;
	ChangDmand(ser);
	for (int i = 0; i < ser.size(); i++)
	   totaldemand += ser[i].d;
	cout << "total demand:" << totaldemand << endl;
	cout << "service size" << ser.size()<< endl;
	cout <<"graph size:"<<G.m<< endl;
	cout<<"node:"<<NODE<<endl;
	cout<<"task:"<<Task<<endl;
	cout<<"edge:"<<EDge<<endl;
	cout<<"type:"<<TYPE<<endl;
	cout<<"graph type:"<<GRAPHTYPE<<endl;
	cout<<"capacity:"<<CAPACITY<<endl;
	ofstream outfile(INFOFILE,ios::app);
	for(int i=0;i<args;i++)
	{
	  switch (arg[i][0])
	  {
		  case 'L':
			  {Lag_Parrel(G,ser,outfile);break;}
		  case 'S':
			  {Lag_Serial(G,ser,outfile);break;}
		  case 'G':
			  {GA_Parrel(G,ser,outfile);break;}
		  case 'T':
			  {GA_Serial(G,ser,outfile);break;}
		  case 'C':
			  {Cplexsolve(G,ser,outfile);break;}
		  case'J':
		  {Pathcheck(G,ser);break;}

	  }
	}
	return 0;
}
