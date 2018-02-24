/*************************************************
Copyright:UESTC
Author: ZhangQian
Date:2016-08-25
Description:GA-SROA method
**************************************************/
#include"Graph.h"
#include "service.h"
#include"taskPath.h"
#include"valuemark.h"
#include"curand_kernel.h"
#include"iostream"
#include <fstream>
#include"const.h"
#include<algorithm>
#include<vector>
#include<math.h>
#include"BFS.h"
#include"time.h"
#include"PathArrange.h"
using namespace std;
#define power .5
#define NNN 10000
float distan[NNN];
int pedge[NNN];
struct FitVM {
	float value;
	int mark;
	FitVM(float _value=0,float _mark=0){
		value = _value;
		mark = _mark;
	}
};
class NewGA
{
	private:
	taskPath* PathSets;
	Graph &G;/*Network topo*/
	int*st;/*start node array*/
	int*te;/*destination node array*/
	float*demand;/*demand bandwidth array*/
	int**chormes;/*chromosomes(feasible solution set)*/
	int**childs;/*middle chromosomes*/
	int**monsters;/*mutation chromosomes*/
	float*capacity;/*link capacities*/
	float best;/*best objective*/
	int totalweight;
	vector<FitVM> Fits;/*chromosomes evaluation array*/
	vector<pair<int,vector<int>>> Result;/*result routes*/
	vector<pair<float,int>>answers;
	public:
		NewGA( Graph &_G):G(_G){
			st = (int*)malloc(Task*sizeof(int));
			te = (int*)malloc(Task*sizeof(int));
			demand = (float*)malloc(Task*sizeof(float));
			capacity = (float*)malloc(sizeof(float)*EDge);
			for (int i = 0; i < EDge; i++)
				capacity[i] = G.incL[i].capacity;
			chormes = (int**)malloc(sizeof(int*)*pop);
			childs = (int**)malloc(sizeof(int*)*Beta);
			monsters = (int**)malloc(sizeof(int*)*Gama);
			for (int i = 0; i < pop; i++)
				chormes[i] = (int*)malloc(sizeof(int)*Task);
			for (int i = 0; i < Beta; i++)
				childs[i] = (int*)malloc(sizeof(int)*Task);
			for (int i = 0; i<Gama; i++)
				monsters[i] = (int*)malloc(sizeof(int)*Task);
		};
		~NewGA(){
			free(st);
			free(te);
			free(demand);
			free(capacity);
			free(chormes);
			free(childs);
			free(monsters);
		}
	public:
		/*************************************************
		Function:GAsearch
		Description:GA iterations.
		*************************************************/
		vector<pair<string,float> > GAsearch(vector<service>&ser, taskPath*_PathSets,ofstream&Out){
			cout<<"GA Serial searching......."<<endl;
			float start = float(1000*clock())/ CLOCKS_PER_SEC;
			for (int i = 0; i < Task; i++)
			{
				st[i] = ser[i].s;
				te[i] = ser[i].t;
				demand[i] = ser[i].d;
			}
			PathSets=_PathSets;
			/*initialize feasible chromosome solutions*/
			GoldnessMake();
			int round = 1;
			int cow = 0;
			best =100*INFHOPS*Task;
			int iter = 0;
			vector<float>middata;
			int mkd=2;
			while (true)
			{  
				iter++;
				/* cross parents chromosomes to get children*/
				cross();
				mutation();
				reload();
				int value=evaluate();
				if (value<best)
				{
					mkd--;
					best = value;
					cow =0;
				}
				middata.push_back(value);
				cow++;
				if(mkd>0&&cow<100)
					continue;
				time_t time=1000*clock()/ CLOCKS_PER_SEC;
				if (cow >loomore||((time-start)>EXPIRE&&GANOEX<0))
					break;
				round++;
			}
			float end =float(1000*clock())/ CLOCKS_PER_SEC;
			Out<<"run info for GA serial:"<<"node:"<<NODE<<"edge"<<EDge<<"task:"<<Task<<"capacity"<<CAPACITY<<endl;
			float lowbound=0;
			for(int i=0;i<Task;i++)
				lowbound+=demand[i]*INFHOPS;
			pair<float,int>md=more();
			CheckR(&G,Result,ser,string("GA_Serial"));
			float gap=middata[middata.size()-1]-best;
                        for(int i=0;i<middata.size();i++)
                                middata[i]-=gap;
			vector<pair<string,float>> rdata;
			writejsoniter(GASFILE,middata,string("GA_Serial"));
			rdata.push_back(make_pair(string("object"),best));
			rdata.push_back(make_pair(string("inf_obj"),lowbound));
			rdata.push_back(make_pair(string("task_add_in"),md.second));
			rdata.push_back(make_pair(string("flow_add_in"),md.first));
			rdata.push_back(make_pair(string("total_weight"),totalweight));
			rdata.push_back(make_pair(string("time"),(end-start)));
			rdata.push_back(make_pair(string("iter_num"),iter));
			rdata.push_back(make_pair(string("iter_time"),(float)(end-start)/(float)iter));
			rdata.push_back(make_pair(string("gap"),gap));
			writejsondata(DATAFILE,rdata,string("GA_Serial"));
			return rdata;
		};
	private:
		bool static randcmp(pair<float, pair<int,int>>a, pair<float,pair<int,int>>b)
		{
			if (a.first>b.first)
				return true;
			return false;
		};
		bool static FitCmp(FitVM a, FitVM b)
		{
			if (a.value<b.value)
				return true;
			return false;
		};
		/*************************************************
		Function:GoldnessMake
		Description:generate initial feasible chromosomes.
		*************************************************/
		void GoldnessMake(){
			for (int i = 0; i < pop; i++)
			{
				float flows = 0;
				vector<pair<float,pair<int,int>>> OoO;
				for (int j = 0; j < Task; j++)
				{
					int round = rand() % PathSets[j].num;
					int k = 0;
					while (PathSets[j].Pathset[round][k]>0)k++;
					float value = demand[j] /pow(k,0.5);
					OoO.push_back(make_pair(value, make_pair(j,round)));
				}
				sort(OoO.begin(), OoO.end(), randcmp);
				vector<int> tmpc(EDge, 0);
				for (int j = 0; j < Task; j++)
				{
					int OjO = OoO[j].second.first;
					int gene = OoO[j].second.second;
					int k = 0;
					int flag = 0;
					while (true)
					{
						int edge = PathSets[OjO].Pathset[gene][k];
						if (edge < 0)
							break;
						if (tmpc[edge] + demand[OjO]>capacity[edge])
						{
							flag = 1;
							break;
						}
						k++;
					}
					if (flag == 0)
					{
						int k = 0;
						while (true)
						{
							int edge = PathSets[OjO].Pathset[gene][k];
							if (edge < 0)
								break;
							tmpc[edge] += demand[OjO];
							k++;
						}
						chormes[i][OjO] = gene;
						flows += demand[OjO];

					}
					else
					{
						int rat = rand() % 10;
						if (rat < 10)
							chormes[i][OjO] = -1;
						else
							chormes[i][OjO] = rand() % PathSets[i].num;

					}
				}
				Fits.push_back(FitVM(flows, i));
			}
			sort(Fits.begin(), Fits.end(), FitCmp);

		};
		/*************************************************
		Function:evaluate
		Description:evaluate all chromosomes and return the best objective of this iteration.
		*************************************************/
		int  evaluate(){
			for (int i = 0; i < pop; i++)
			{
				float flows = 0;
				vector<float>tmpc(EDge, 0);
				vector<int>ifchao(EDge, 0);
				for (int j = 0; j < Task; j++)
				{
					int choose = chormes[i][j];
					if (choose < 0)
						{
						flows += demand[j]*INFHOPS;
						continue;}
					int k = 0;
					while (true){
						int edge = PathSets[j].Pathset[choose][k];
						if (edge < 0)
							break;
						tmpc[edge] += demand[j];
						if (tmpc[edge]>capacity[edge])
							ifchao[edge] = 1;
						k++;
					}
					flows += demand[j]*k;
				}
				for (int j = 0; j < EDge; j++)
				{
					flows += 100*Task*INFHOPS*ifchao[j];
				}
				Fits[i].value = flows;
				Fits[i].mark = i;
			}
			sort(Fits.begin(), Fits.end(), FitCmp);
			return Fits[0].value;
		};
		/*cross parents chromosomes to get children*/
		void cross(){
			for (int i = ALPHA; i < ALPHA + Beta; i+=2)
			{
				int father = Fits[rand() % (ALPHA)].mark;//Fits[getparents()].mark;
				int monther = Fits[rand()%(ALPHA+Beta)].mark;
				for (int j = 0; j < Task; j++)
				{
					int divider = rand() % 2;
					if (divider < 1)
					{
						childs[i+1-ALPHA][j] = chormes[father][j];
						childs[i-ALPHA][j] = chormes[monther][j];
					}
					else
					{
						childs[i-ALPHA][j] = chormes[father][j];
						childs[i+1-ALPHA][j] = chormes[monther][j];
					}
				}
			}
		}
		void changecross(){
			for (int i = ALPHA; i < ALPHA + Beta; i += 2)
			{
				int father = Fits[rand() % ALPHA].mark;
				int monther = Fits[rand() % (ALPHA + Beta)].mark;
				int position = rand() % Task;
				for (int j = 0; j < position; j++)
				{
					childs[i + 1 - ALPHA][j] = chormes[father][j];
					childs[i - ALPHA][j] = chormes[monther][j];
				}
				for (int j = position; j < Task; j++)
				{
					childs[i + 1 - ALPHA][j] = chormes[monther][j];
					childs[i - ALPHA][j] = chormes[father][j];
				}
			}
		}
		/*randomly mutate some chromosomes*/
		void mutation(){
			for (int i = 0; i < Gama; i++)
			{
				//int ri = i%(Gama);
				int muone = rand() % (ALPHA+Beta+Gama);
				int muposition = rand() % Task;
				int newv=rand() % (PathSets[muposition].num+1);
				if (newv == PathSets[muposition].num)
					newv = -1;
				for (int j = 0; j < Task; j++)
					monsters[i][j] =chormes[muone][j];
				monsters[i][muposition] = newv;

			}
		}
		void reload(){
			for (int i = ALPHA; i < ALPHA + Beta; i++)
			{
				int fiti = Fits[i].mark;
				for (int j = 0; j < Task; j++)
					chormes[fiti][j] = childs[i - ALPHA][j];
			}
			for (int i = ALPHA + Beta; i < pop; i++)
			{
				int fiti = Fits[i].mark;
				for (int j = 0; j < Task; j++)
					chormes[fiti][j] = monsters[i - ALPHA - Beta][j];
			}
		}
		pair<float,int> more(){
			float totalv = Fits[0].value;
			int bestchoice = Fits[0].mark;
			float tf=0;
			totalweight=0;
			vector<int>remain;
			for (int i = 0; i < Task; i++)
			{
				
				int route = chormes[bestchoice][i];
				if (route < 0)
					remain.push_back(i);
				else{
					int k = 0;
					vector<int> path;
					while (true){
						int svalue = PathSets[i].Pathset[route][k];
						if (svalue < 0)
							break;
						path.push_back(svalue);
						capacity[svalue] -= demand[i];
						k++;
					}
					reverse(path.begin(),path.end());
					Result.push_back(make_pair(i,path));
					totalweight+=k;
					tf+=demand[i];
					answers.push_back(make_pair(demand[i],k));

				}
			}
			int stillremain=0;
			for (int i = 0; i < remain.size(); i++)
			{
				BFS(&G, st[remain[i]], te[remain[i]], distan, pedge, demand[remain[i]],capacity);
				int f = pedge[te[remain[i]]];
				if (distan[te[remain[i]]]<INFHOPS)
				{
					totalv += demand[remain[i]]*(distan[te[remain[i]]]-INFHOPS);
					int j = 0;
					vector<int> route;
					while (f >= 0)
					{
						j++;
						capacity[f] -=demand[remain[i]];
						route.push_back(f);
						f = pedge[G.incL[f].tail];
						
					}
					Result.push_back(make_pair(remain[i],route));
					totalweight+=distan[te[remain[i]]];
					tf+=demand[remain[i]];
					answers.push_back(make_pair(demand[remain[i]],j));
				}
				else
					stillremain++;
			}
			best=totalv;
			cout<<Result.size()<<" "<<answers.size()<<endl;
			return make_pair(tf,Task-stillremain);
		}
		int getparents(){
			int dd = rand() % 100;
			if (dd < 50)
				return rand() % ALPHA;
			if (dd < 80)
				return rand() % Beta + ALPHA;
			if (dd < 100)
				return rand() % Gama + ALPHA + Beta;
		}
};

