/*************************************************
Copyright:UESTC
Author: ZhangQian
Date:2016-08-25
Description:cplex solve
**************************************************/
#include"CplexM.h"
#include <ilcplex/ilocplex.h>
#include<ilcplex/cplex.h>
#include"const.h"
#include"algorithm"
#include"PathArrange.h"
using namespace std;
vector<float>middata;
float lowbound;
ILOMIPINFOCALLBACK5(loggingCallback,
                    IloNumVarArray, vars,
                    IloNum,         lastLog,
                    IloNum,         lastIncumbent,
                    IloNum,         startTime,
                    IloNum,         startDetTime)
{
   int newIncumbent = 0;
   int nodes = getNnodes();

   if ( hasIncumbent()) {
      //cout<<lowbound+getIncumbentObjValue()<<endl;
	  middata.push_back(lowbound+getIncumbentObjValue());
   }

}
/*************************************************
Function:CplexM
Description:Network data preprocess
*************************************************/
CplexM::CplexM(vector<service>&ser, taskPath*_PathSets, Graph &G, ofstream&_Out):Gm(G),serv(ser),constraint(EDge,vector<pair<float,int>>())
{
	for(int i=0;i<Task;i++)
		demand.push_back(ser[i].d);
	for(int i=0;i<G.m;i++)
		capacity.push_back(G.incL[i].capacity);
	PathSets=_PathSets;
	int vcount=-1;
	for(int i=0;i<Task;i++)
	{
		int n=PathSets[i].num;
		vector<pair<float,int>>newcon;
		for(int j=0;j<n;j++)
		{
			variables.push_back(++vcount);
			va2d[vcount]=i;
			va2p[vcount]=j;
			newcon.push_back(make_pair(1,vcount));
			int k=0;
			while(true)
				{
				 int ed=PathSets[i].Pathset[j][k];
				 if(ed<0)
					 break;
				 k++;
				 constraint[ed].push_back(make_pair(demand[i],vcount));
				}
			object.push_back(make_pair(demand[i]*(k-INFHOPS),vcount));
		}
		constraint.push_back(newcon);
		capacity.push_back(1);
	}
	lowbound=0;
	for(int i=0;i<Task;i++)
		lowbound+=INFHOPS*demand[i];
};
/*************************************************
Function:solve
Description:build the model and solve by cplex.
*************************************************/
pair<int,vector<pair<string,float>>> CplexM::solve(){
	IloEnv env;
        IloModel model(env);
	IloNumVarArray x(env);
	IloRangeArray con(env);
	IloObjective obj = IloMinimize(env);
	time_t st=clock();
	cout<<"here 1"<<endl;
	for(int i=0;i<variables.size();i++)
		{
		x.add(IloNumVar(env,0,1,ILOINT));}
	for(int i=0;i<object.size();i++)
	   {//if((clock()-st)/CLOCKS_PER_SEC>500000)
			//return;
		obj.setLinearCoef(x[object[i].second],object[i].first);
	   }
	for(int i=0;i<capacity.size();i++)
	{ //if((clock()-st)/CLOCKS_PER_SEC>500000)
		//return;
		con.add(IloRange(env, -IloInfinity,capacity[i]));}
	for(int i=0;i<constraint.size();i++)
		for(int j=0;j<constraint[i].size();j++)
			con[i].setLinearCoef(x[constraint[i][j].second],constraint[i][j].first);

	model.add(obj);
	model.add(con);
	IloCplex cplex(model);
	cout<<"here now"<<endl;
	cplex.exportModel("lpex4.lp");
	//cplex.setOut(env.getNullStream());
	IloNum lastObjVal = (obj.getSense() == IloObjective::Minimize ) ?
								   IloInfinity : -IloInfinity;
	cplex.use(loggingCallback(env,x, -100000, lastObjVal,
									   cplex.getCplexTime(), cplex.getDetTime()));
	IloNum timelimit=(EXPIRE/1000);
	cplex.setParam(IloCplex::TiLim,timelimit);
	//cplex.setParam(IloCplex::ClockType,1);
	float start=1000*cplex.getTime();
	if ( !cplex.solve() ) {
		 env.error() << "Failed to optimize LP" << endl;
		 throw(-1);
	  }
	float  end=1000*cplex.getTime();
	//time_t eee=float(1000*clock())/ CLOCKS_PER_SEC;
	IloNumArray vals(env);
	cplex.getValues(vals,x);
	int count=0;
	float tf=0;
	int totalhop=0;
	float checkSS=0;
	float subpart=lowbound;
	vector<pair<float,int>> answers;
	vector<pair<int,vector<int>>>Result;
	for(int i=0;i<variables.size();i++)
		if(float(vals[i])>0.9)
			{
				vector<int> path;
				count++;
				tf+=demand[va2d[i]];
				totalhop+=PathSets[va2d[i]].Pathset[va2p[i]].size()-1;
				checkSS+=demand[va2d[i]]*(PathSets[va2d[i]].Pathset[va2p[i]].size()-1);
				answers.push_back(make_pair(demand[va2d[i]],PathSets[va2d[i]].Pathset[va2p[i]].size()-1));
				for(int k=0;k<PathSets[va2d[i]].Pathset[va2p[i]].size()-1;k++)
					path.push_back(PathSets[va2d[i]].Pathset[va2p[i]][k]);
				reverse(path.begin(),path.end());
				Result.push_back(make_pair(va2d[i],path));
			}
	CheckR(&Gm,Result,serv,string("Cplex_solve"));
	float objv=cplex.getObjValue();
	float bound=cplex.getBestObjValue();
	middata.push_back(objv+lowbound);
	writejsoniter(LAGPFILE,middata,string("Cplex_solve"));
	vector<pair<string,float>> rdata;
	rdata.push_back(make_pair(string("object"),objv+lowbound));
	rdata.push_back(make_pair(string("inf_obj"),lowbound));
	rdata.push_back(make_pair(string("bound"),bound+lowbound));
	rdata.push_back(make_pair(string("task_add_in"),count));
	rdata.push_back(make_pair(string("flow_add_in"),tf));
	rdata.push_back(make_pair(string("total_weight"),totalhop));
	rdata.push_back(make_pair(string("iter_num"),middata.size()));
	rdata.push_back(make_pair(string("status"),cplex.getStatus()));
	return make_pair(middata.size(),rdata);
}

