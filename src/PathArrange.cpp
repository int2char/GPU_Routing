/*************************************************
Copyright:UESTC
Author: ZhangQian
Date:2016-08-25
Description:path adjustment method
**************************************************/
#include"Graph.h"
#include<vector>
#include<time.h>
#include <algorithm>
#include<fstream>
#include"BFS.h"
#include"stdio.h"
#include"dijkstra.h"
#include <iostream>
#include"routemask.h"
#include"const.h"
#include"math.h"
#include<fstream>
#include<map>
#include<iomanip>
#include"service.h"
#define SQ .5
#define	RO 80
#define MU 0
#define BON 1
#define N 10000
float dist[N];
int peg[N];
float bestflow;
float cbbflow;
vector<float> canswer;
void writejsondanswer(vector<pair<float,int>>&data,string method)
{
	ofstream midfile(ANSWERS,ios::app);
	midfile<<"{"<<"\"type\":"<<"\""<<TYPE<<"\""<<",";
	midfile<<"\"graph_type\":"<<"\""<<GRAPHTYPE<<"\""<<",";
	midfile<<"\"node\":"<<"\""<<NODE<<"\""<<",";
	midfile<<"\"edge\":"<<"\""<<EDge<<"\""<<",";
	midfile<<"\"task\":"<<"\""<<Task<<"\""<<",";
	midfile<<"\"capacity\":"<<"\""<<CAPACITY<<"\""<<",";
	midfile<<"\"method\":"<<"\""<<method<<"\""<<",";
	midfile<<"\"flows\":"<<"\"";
	for(int i=0;i<data.size();i++)
		midfile<<data[i].first<<" ";
	midfile<<"\""<<",";
	midfile<<"\"hops\":"<<"\"";
	for(int i=0;i<data.size();i++)
		midfile<<data[i].second<<" ";
	midfile<<"\"";
	midfile<<"}"<<endl;
	midfile.close();
}

bool cmpv(RouteMark a1, RouteMark a2){
	if (a1.length >a2.length)
		return true;
	return false;
}

/*************************************************
Function:rearrange
Description:path adjustment realization for LR-SROA.
G:Graph struct of the network.
capacity:link capacity array.
lambda:weight penalty array.
pre:precursor node array.
d:distance array.
pd:service demand array.
te:destination array of services.
st:start node array of services.
num:service number.
mum:edge number.
optimal:record the best object value.
stillS:the number of services that can't add to network of current iteration.
wide&len:precursor node array dimension.
wide:wid=1
len:len=NODE
StoreRoute:store the routes.
BestRoute:record the routes with the best object value.
*************************************************/
float  rearrange(Graph* G,float*capacity,float *lambda,int*pre, float*d, float *pd, int *te, int *st,int* mask,double& optimal, int&stillS, int wide, int len, vector<vector<int>>&StoreRoute, vector<vector<int>>&BestRoute)
{
	int num=Task;
	int mum=EDge;
	int totalflow=0;
	vector<int> remain;
	vector<RouteMark> Routes;
	int count=0;
	/*reload capacity*/
	for (int i = 0; i<G->m; i++)
		capacity[i]=G->incL[i].capacity;
	/*count the demand bandwith and route hops*/
	for (int i = 0; i < num; i++)
	{
		int n = 0;
		int f = pre[te[i] * wide + i*len];
		if (StoreRoute[i][0] < 0)
		{
			while (f >= 0){
				n++;
				f = pre[G->incL[f].tail*wide + i*len];
				if (n>1000)
					cout << "circle"<<i<< endl;
			}
		}
		else
			n = StoreRoute[i][0];
		float value = pow(pd[i], SQ) / ((float)n);
		Routes.push_back(RouteMark(value, i));
	}
	/*ranking the services in decreasing order*/
	sort(Routes.begin(), Routes.end(), cmpv);
	/*add the service to network ,if the service can't be added to the network,record the service ID in remain array */
	for (int ai = 0; ai <Routes.size(); ai++)
	{
		int i = Routes[ai].mark;
		float demand = pd[i];
		int f = pre[te[i] * wide + i*len];
		int n = 0;
		int flag = 0;
		if (StoreRoute[i][0] < 0)
		{
			while (f >= 0)
			{
				if (capacity[f] < demand)
				{
					flag = 1;
					break;
				}
				if (n > 1000)
				{
					printf("circle!!!in s:%d:\n", i);
				}
				f = pre[G->incL[f].tail*wide + i*len];
				n++;
			}
			if(n>=INFHOPS)
				flag=1;
			if (flag == 0)
			{
				StoreRoute[i].clear();
				StoreRoute[i].push_back(n);
				int j = 0;
				totalflow += demand*n;
				count++;
				int f = pre[te[i] * wide + i*len];
				while (f >= 0)
				{
					j++;
					StoreRoute[i].push_back(f);
					capacity[f] -= demand;
					f = pre[G->incL[f].tail*wide + i*len];
					if (n > 10100)
						printf("erro2\n");
				}
				StoreRoute[i].push_back(-1);
			}
			else
			{
				totalflow+=demand*INFHOPS;
				count++;
				remain.push_back(i);
				StoreRoute[i].clear();
				StoreRoute[i].push_back(-1);
			}
		}
		else
		{
			int j = 0;
			while (true)
			{
				j++;
				int edn = StoreRoute[i][j];
				if (edn < 0)
					break;
				if (capacity[edn] < demand)
				{
					flag = 1;
					break;
				}
			}
			if(j-1>=INFHOPS)
				flag=1;
			if (flag == 0)
			{
				j = 0;
				while (true)
				{
					j++;
					int edn = StoreRoute[i][j];
					if (edn < 0)
						break;
					capacity[edn] -= demand;
				}
				totalflow += demand*(j-1);
				count++;
			}
			else
			{
				StoreRoute[i].clear();
				StoreRoute[i].push_back(-1);
				totalflow+=demand*INFHOPS;
				count++;
				remain.push_back(i);
			}
		}

	}
	time_t trs = clock();
	/*find routes for the remaining services using BFS
	 for those remaining services that we can't find satisfing routes via BFS,
	 we will add penalty weight on their routes*/
	stillS = remain.size();
			if (BON>0)
			{
				int ren = remain.size();
				vector<int> stillremain;
				for (int i = 0; i < ren; i++){
					float demand = pd[remain[i]];
					BFS(G, st[remain[i]], te[remain[i]], dist, peg, demand, capacity);
					int f = peg[te[remain[i]]];
					if (dist[te[remain[i]]]<INFHOPS)
					{
						StoreRoute[remain[i]].clear();
						StoreRoute[remain[i]].push_back(dist[te[remain[i]]]);
						int j = 0;
						while (f >= 0)
						{

							if (capacity[f] < demand)
								printf("erro!\n");
							capacity[f] -= demand;
							StoreRoute[remain[i]].push_back(f);
							f = peg[G->incL[f].tail];
							j++;
						}
						StoreRoute[remain[i]].push_back(-1);
						totalflow += demand*(j-INFHOPS);
					}
					else
					{
						stillremain.push_back(remain[i]);
						StoreRoute[remain[i]].clear();
						StoreRoute[remain[i]].push_back(-1);
					}

				}
				stillS = stillremain.size();
			}
	float tflow = totalflow;
	/*update best result*/
	if (tflow<optimal)
	{
		optimal = tflow;
		cbbflow = optimal;
		for (int i = 0; i < num; i++)
		{
			BestRoute[i].clear();
			int j = 0;
			while (true)
			{
				BestRoute[i].push_back(StoreRoute[i][j]);
				if (StoreRoute[i][j] < 0)
					break;
				j++;
			}
		}
	}
	int maskC = 0;
	for (int i = 0; i < num; i++)
	{
		if (StoreRoute[i][0] < 0)
			mask[maskC++] = i;
		else
		{
			int random = rand() % 10;
			if (random < MU)
			{
				mask[maskC++] = i;
				StoreRoute[i].clear();
				StoreRoute[i].push_back(-1);
			}
		}
	}
	/*for those services that can't add to the network,caculate the penalty weight for their routes*/
	for (int i = 0; i < stillS; i++)
	{
		float demand = pd[remain[i]];
		int f = pre[te[remain[i]] * wide + remain[i] * len];
		int max = 0;
		int mm = -1;
		while (f >= 0)
		{
			if (demand>capacity[f]);
			{
				int r = rand() % 90;
				if (r>RO)
					lambda[f] += 1;
			}
			f = pre[G->incL[f].tail*wide + remain[i] * len];
		}
	}
	stillS = maskC;
	return tflow;
}
/*************************************************
Function:rearrange2
Description:path adjustment realization for LR-PROA.
G:Graph struct of the network.
capacity:link capacity array.
lambda:weight penalty array.
pre:precursor node array.
d:distance array.
pd:service demand array.
te:destination array of services.
st:start node array of services.
num:service number.
mum:edge number.
optimal:record the best object value.
stillS:the number of services that can't add to network of current iteration.
wide&len:precursor node array dimension.
wide:wide=num.
len:wide=1.
StoreRoute:store the routes.
BestRoute:record the routes with the best object value.
*************************************************/
float rearrange2(Graph* G, float *capacity, float *lambda, int*pre, float*d, float *pd, int *te, int *st,int* mask,double& optimal, int&stillS, int wide, int len, vector<vector<int>>&StoreRoute, vector<vector<int>>&BestRoute)
{
	int num=Task;
	int mum=EDge;
	int totalflow=0;
	vector<RouteMark> Routes;
	vector<int>vecmask;
	/*reload capacity*/
	for (int i = 0; i < mum; i++)
	{
		capacity[i] = G->incL[i].capacity;
	}
	vector<int> remain;
	/*count the demand bandwith and route hops*/
	for (int i = 0; i < num; i++)
	{
		int n = 0;			
		int f = pre[te[i] * wide + st[i]*len];
		if (StoreRoute[i][0] < 0)
		{
			while (f >= 0){
				n++;
				f = pre[G->incL[f].tail*wide + st[i]*len];
				if (n>1000)
					cout << "circle"<<i<< endl;
			}
		}
		else
			n = StoreRoute[i][0];
		float value = pow(pd[i], SQ) / ((float)n);
		Routes.push_back(RouteMark(value, i));
	}
	/*ranking the services in decreasing order*/
	sort(Routes.begin(), Routes.end(), cmpv);
	/*add the service to network ,if the service can't be added to the network,record the service ID in remain array */
	for (int ai = 0; ai <Routes.size(); ai++)
	{
		int i = Routes[ai].mark;
		float demand = pd[i];
		int f = pre[te[i] * wide + st[i]*len];
		int n = 0;
		int flag = 0;
		if (StoreRoute[i][0] < 0)
		{
			
			while (f >= 0)
			{
				if (capacity[f] < demand)
				{
					flag = 1;
					break;
				}
				if (n > 1000)
				{
					printf("circle!!!in s:%d:\n", i);
				}
				f = pre[G->incL[f].tail*wide + st[i]*len];
				n++;
			}
			if(n>=INFHOPS)
				flag=1;
			if (flag == 0)
			{
				StoreRoute[i].clear();
				StoreRoute[i].push_back(n);
				int j = 0;
				totalflow += demand*n;
				int f = pre[te[i] * wide + st[i]*len];
				while (f >= 0)
				{
					j++;
					StoreRoute[i].push_back(f);
					capacity[f] -= demand;
					f = pre[G->incL[f].tail*wide + st[i]*len];
					if (n > 10100)
						printf("erro2\n");
				}
				StoreRoute[i].push_back(-1);
				//printf("\n");
			}
			else
			{
				totalflow+=demand*INFHOPS;
				remain.push_back(i);
				StoreRoute[i].clear();
				StoreRoute[i].push_back(-1);
			}
		}
		else
		{
			int j = 0;
			while (true)
			{
				j++;
				int edn = StoreRoute[i][j];
				if (edn < 0)
					break;
				if (capacity[edn] < demand)
				{
					flag = 1;
					break;
				}
			}
			if(j-1>=INFHOPS)
				flag=1;
			if (flag == 0)
			{
				j = 0;
				while (true)
				{
					j++;
					int edn = StoreRoute[i][j];
					if (edn < 0)
						break;
					capacity[edn] -= demand;
				}
				totalflow += demand*(j-1);
			}
			else
			{
				StoreRoute[i].clear();
				StoreRoute[i].push_back(-1);
				totalflow+=demand*INFHOPS;
				remain.push_back(i);
			}
		}

	}
	time_t trs = clock();
	/*find routes for the remaining services using BFS
		 for those remaining services that we can't find satisfing routes via BFS,
		 we will add penalty weight on their routes*/
	stillS = remain.size();
			if (BON>0)
			{
				int ren = remain.size();
				vector<int> stillremain;
				for (int i = 0; i < ren; i++){
					float demand = pd[remain[i]];
					BFS(G, st[remain[i]], te[remain[i]], dist, peg, demand, capacity);
					int f = peg[te[remain[i]]];
					if (dist[te[remain[i]]]<INFHOPS)
					{
						StoreRoute[remain[i]].clear();
						StoreRoute[remain[i]].push_back(dist[te[remain[i]]]);
						int j = 0;
						while (f >= 0)
						{

							if (capacity[f] < demand)
								printf("erro!\n");
							capacity[f] -= demand;
							StoreRoute[remain[i]].push_back(f);
							f = peg[G->incL[f].tail];
							j++;
						}
						StoreRoute[remain[i]].push_back(-1);
						totalflow += demand*(j-INFHOPS);
					}
					else
					{
						stillremain.push_back(remain[i]);
						StoreRoute[remain[i]].clear();
						StoreRoute[remain[i]].push_back(-1);
					}

				}
				stillS = stillremain.size();
			}
	float tflow = totalflow;
	/*update best result*/
	if (tflow<optimal)
	{
		optimal = tflow;
		cbbflow = optimal;
		for (int i = 0; i < num; i++)
		{
			BestRoute[i].clear();
			int j = 0;
			while (true)
			{
				BestRoute[i].push_back(StoreRoute[i][j]);
				if (StoreRoute[i][j] < 0)
					break;
				j++;
			}
		}
	}
	for (int i = 0; i < num; i++)
	{
		if (StoreRoute[i][0] < 0)
			vecmask.push_back(st[i]);
		else
		{
			int random = rand() % 10;
			if (random < MU)
			{
				vecmask.push_back(st[i]);
				StoreRoute[i].clear();
				StoreRoute[i].push_back(-1);
			}
		}
	}
	/*for those services that can't add to the network,caculate the penalty weight for their routes*/
	for (int i = 0; i < stillS; i++)
	{
		float demand = pd[remain[i]];
		int f = pre[te[remain[i]] * wide + st[remain[i]] * len];
		int max = 0;
		int mm = -1;
		while (f >= 0)
		{

			if (demand>capacity[f]);
			{
				int r = rand() % 90;
				if (r>RO)
					lambda[f] += 1;
			}
			f = pre[G->incL[f].tail*wide + st[remain[i]] * len];
		}
	}
	/*in bellman ford alg,we can get the routes of soure s to all the other nodes,so if some demands has the same source node,we can merge the demands into one*/
	/*so we unique the sources as follow*/
	vector<int>::iterator end=unique(vecmask.begin(),vecmask.end());
	sort(vecmask.begin(),end);
	vector<int>::iterator begin=vecmask.begin();
	stillS=0;
	for(begin;begin<end;begin++)
		mask[stillS++]=*begin;
	return tflow;
}
float morein(Graph* G, float capacity[], float pd[], int te[], int st[], int num, int mum,vector<vector<int>>&StoreRoute,ostream& Out, vector<RouteMark>& bestroutes){
	float totalflow = 0;
	int max = 100*Task*INFHOPS;
	for (int i = 0; i < canswer.size(); i++)
	{
		if (i % 10 == 0)
			Out << endl;
		if (canswer[i]<max)
			Out << "(";
		Out << canswer[i];
		if (canswer[i]<max)
		{
			Out << ")";
			max = canswer[i];
		}
		Out << " ";
	}
	if (BON == 0){
		for (int i = 0; i < mum; i++)
		{
			capacity[i] = G->incL[i].capacity;
		}
		vector<int>remain;
		for (int ai = 0; ai <bestroutes.size(); ai++)
		{
			int i = bestroutes[ai].mark;
			float demand = pd[i];
			int n = 0;
			if (StoreRoute[i][0]>0)
			{
				
				int j = 0;
				while (true)
				{
					j++;
					int edn = StoreRoute[i][j];
					if (edn < 0)
						break;
					capacity[edn] -= demand;
				}
				totalflow += demand*j;

			}
			else
				remain.push_back(i);
		}

		{
			int ren = remain.size();
			for (int i = 0; i < ren; i++){
				float demand = pd[remain[i]];
				BFS(G, st[remain[i]], te[remain[i]], dist, peg, demand, capacity);
				int f = peg[te[remain[i]]];
				int ran = rand() % 10;
				if (dist[te[remain[i]]] <10)
				{
					StoreRoute[remain[i]][0] = dist[te[remain[i]]];
					int j = 0;
					while (f >= 0)
					{
						j++;
						if (capacity[f] < demand)
							printf("erro!\n");
						capacity[f] -= demand;
						StoreRoute[remain[i]][j] = f;
						f = peg[G->incL[f].tail];
					}
					StoreRoute[remain[i]][++j] = -1;
					totalflow += demand*j;
				}
				else
				{
					StoreRoute[remain[i]][0] = -1;
					totalflow+=demand*INFHOPS;
				}

			}
		}
		Out <<endl<< "esmate gap is:" << (totalflow-cbbflow) << endl;
		Out << "esmate result is:" << endl;
		max = 100 * Task*INFHOPS;
		for (int i = 0; i < canswer.size(); i++)
		{
			if (i % 10 == 0)
				Out << endl;
			if (canswer[i] < max)
				Out << "(";
			Out << canswer[i] + (totalflow-cbbflow);
			if (canswer[i] <max)
			{
				Out << ")";
				max = canswer[i];
			}
			Out << " ";

		}
	}
	return totalflow;
}
/*************************************************
Function:GetResult
Description:routes result process
*************************************************/
vector<pair<int, vector<int> > > GetResult(Graph* G, int st[], int te[], float pd[], int pre[], int num, int mum, int wide, int len)
{
	vector<pair<int, vector<int> > > result;
	float capacity[10020];
	for (int i = 0; i < mum; i++)
	{
		capacity[i] = G->incL[i].capacity;
	}
	vector<int> remain;
	for (int i = 0; i <num; i++)
	{
		float demand = pd[i];
		int f = pre[te[i] * wide + i*len];
		int n = 0;
		int flag = 0;

		while (f >= 0)
		{
			n++;
			if (capacity[f] < demand)
			{
				flag = 10;
				break;
			}
			if (n > 1000)
			{
				printf("circle!!!in s:%d:\n", i);

			}
			f = pre[G->incL[f].tail*wide + i*len];

		}
		if (flag == 0)
		{
			vector<int>temp;
			int n = 0;
			int f = pre[te[i] * wide + i*len];
			while (f >= 0)
			{
				capacity[f] -= demand;
				temp.push_back(f);
				f = pre[G->incL[f].tail*wide + i*len];
				n++;
				if (n > 10100)
					printf("erro2\n");
			}
			result.push_back(make_pair(i, temp));
		}
		else
			remain.push_back(i);

	}
	int ren = remain.size();

	vector<int> stillremain;
	for (int i = 0; i < ren; i++){
		float demand = pd[remain[i]];
		BFS(G, st[remain[i]], te[remain[i]], dist, peg, demand, capacity);
		//dijcapacity(G, st[remain[i]], te[remain[i]], d, peg, lambda, capacity, demand);
		int f = peg[te[remain[i]]];
		vector<int> temp;
		while (f >= 0)
		{
			capacity[f] -= demand;
			temp.push_back(f);
			f = peg[G->incL[f].tail];
		}
		if (temp.size()>0)
			result.push_back(make_pair(remain[i], temp));
	}
	return result;
}
/*************************************************
Function:GrabResult
Description:routes result process
*************************************************/
vector<pair<int, vector<int>>> GrabResult(vector<vector<int>>&Route, int taskn, int ednum, float* pd){
	vector<pair<int, vector<int> > > result;
	int addin = 0;
	float *capacity = (float*)calloc(ednum, sizeof(float));
	for (int i = 0; i < taskn; i++)
	{
		if (Route[i][0] < 0)
			continue;
		int j = 0;
		addin++;
		vector<int>temp;
		//cout << endl;
		//cout << i << "(" << Route[i][0] <<")"<< ":";
		while (true)
		{
			j++;
			if (Route[i][j] < 0)
				break;
			temp.push_back(Route[i][j]);
			//cout << Route[i][j] << "<-";
		}
		result.push_back(make_pair(i, temp));
		temp.clear();
	}
	return result;
}
/*************************************************
Function:CheckR
Description:check the result
*************************************************/
pair<float,int> CheckR(Graph*G, vector<pair<int, vector<int>> > result,vector<service>&ser,string method)
{
	cout<<"in check"<<endl;
	float lowbound=0;
	for(int i=0;i<Task;i++)
		lowbound+=ser[i].d*INFHOPS;
    float addinpart=0;
	float totalflow=0;
	float checkobject=0;
	float* capacity=new float[G->m];
	int mum = G->m;
	vector<pair<float,int>> answers;
	for (int i = 0; i <mum; i++)
	{
		capacity[i] =G->incL[i].capacity;
	}
	float totalweight = 0;

	for (int i = 0; i < result.size(); i++)
	{

		int flag = 0;
		float demand = ser[result[i].first].d;
		totalflow += demand;
		if (G->incL[result[i].second[0]].head != ser[result[i].first].t)
		{
			cout<<"erro inresult:wrong route/unreach temination/service"<<i<<endl;
			//return;
		}
		//printf("s:%d,t:%d\n", st[result[i].first], te[result[i].first]);
		for (int j = 0; j < result[i].second.size(); j++)
		{
			totalweight += 1;// G->incL[result[i].second[j]].weight;
			capacity[result[i].second[j]] -= demand;
			//printf("%d<-", G->incL[result[i].second[j]].head, G->incL[result[i].second[j]].tail);
			if (capacity[result[i].second[j]] < 0)
			{
				cout << "erro in result!!!:overload edge" << result[i].second[j] << " " << result[i].first << "edge cap :" << capacity[result[i].second[j]]<<"demand"<<demand<<endl;
				//return;
			}
			if (G->incL[result[i].second[j]].tail != ser[result[i].first].s)
				if (G->incL[result[i].second[j]].tail != G->incL[result[i].second[j + 1]].head)
				{
						cout<<"erro inresult:wrong route/unconected route/service"<<result[i].first<<endl;
					//return make_pair(-1,-1);
				}
			if (G->incL[result[i].second[j]].tail == ser[result[i].first].s)
			{
				flag = 1;
			}
		}
		int len=result[i].second.size();
		addinpart+=len*demand;
		if (flag == 0)
		{
				cout<<"erro in result: wrong route/start wrong/service:"<<result[i].first<<endl;
			return make_pair(-1,-1);
		}
		answers.push_back(make_pair(demand,len));
	}
	 for(int i=0;i<result.size();i++)
                        {
			reverse(result[i].second.begin(),result[i].second.end());
                        map<int,int>remap;
                        int first=0;
                        for(int j=0;j<result[i].second.size();j++)
                                {if(first==0)
                                        {
                                        remap.insert(make_pair(G->incL[result[i].second[j]].tail,1));
                                        first=1;
                                        }
                                if(remap.find(G->incL[result[i].second[j]].head)!=remap.end())
                                        cout<<"erro loop here"<<endl;
                                remap.insert(make_pair(G->incL[result[i].second[j]].head,1));
                                }
			remap.clear();
                        }
                        

	cout<<"check corrected!!!"<<endl;
	writejsondanswer(answers,method);
	return make_pair(totalflow,totalweight) ;
}

void CheckRoute(int**Route, int taskn, int ednum, float* pd){
	float *capacity = (float*)calloc(ednum,sizeof(float));
	float tflow = 0;
	int pathnum = 0;
	for (int i = 0; i < taskn; i++)
		{
		if (Route[i][0] < 0)
				continue;
		int j = 0;
		tflow += pd[i];
		while (true)
		{ 
			j++;
			if (Route[i][j] < 0)
				break;
			capacity[Route[i][j]] +=pd[i];
			if (capacity[Route[i][j]]>100)
			{
				cout << "erro ovr capacity!" << endl;
				return;
			}
		}
	}
	cout << "right no erro" << endl<<"totol flow check is:"<<tflow<<endl;
	delete[]capacity;
}
void writejsoniter(char*filename,vector<float>&iter,string method)
{               
                cout<<setprecision(12);
		ofstream midfile(filename,ios::app);
		midfile<<"{"<<"\"type\":"<<"\""<<TYPE<<"\""<<",";
		midfile<<"\"graph_type\":"<<"\""<<GRAPHTYPE<<"\""<<",";
		midfile<<"\"node\":"<<"\""<<NODE<<"\""<<",";
		midfile<<"\"edge\":"<<"\""<<EDge<<"\""<<",";
		midfile<<"\"task\":"<<"\""<<Task<<"\""<<",";
		midfile<<"\"capacity\":"<<"\""<<CAPACITY<<"\""<<",";
		midfile<<"\"method\":"<<"\""<<method<<"\""<<",";
		midfile<<"\"iter\":"<<"\"";
		for(int i=0;i<iter.size();i++)
			midfile<<std::fixed<<iter[i]<<" ";
		midfile<<"\""<<"}"<<endl;
		midfile.close();

}
void writejsondata(char*filename,vector<pair<string,float>>&data,string method)
{
	ofstream midfile(filename,ios::app);
	midfile<<"{"<<"\"type\":"<<"\""<<TYPE<<"\""<<",";
	midfile<<"\"graph_type\":"<<"\""<<GRAPHTYPE<<"\""<<",";
	midfile<<"\"node\":"<<"\""<<NODE<<"\""<<",";
	midfile<<"\"edge\":"<<"\""<<EDge<<"\""<<",";
	midfile<<"\"task\":"<<"\""<<Task<<"\""<<",";
	midfile<<"\"capacity\":"<<"\""<<CAPACITY<<"\""<<",";
	midfile<<"\"method\":"<<"\""<<method<<"\""<<",";
	cout<<setprecision(12);
	float obj=data[0].second;
	float inf_obj=data[1].second;
	//float test=123456578910;
	midfile<<"\"assess\":"<<"\""<<float(obj/inf_obj)<<"\"";
	//data.push_back(make_pair("test:",test));
	for(int i=0;i<data.size();i++)
		{
		midfile<<",";
		midfile<<"\""<<data[i].first<<"\":"<<"\""<<std::fixed<<data[i].second<<"\"";
		}
	midfile<<"}"<<endl;
	midfile.close();
}
