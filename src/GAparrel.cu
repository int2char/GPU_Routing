/*************************************************
Copyright:UESTC
Author: ZhangQian
Date:2016-08-25
Description:GA-PROA method realization
**************************************************/
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include"Graph.h"
#include "service.h"
#include"taskPath.h"
#include"valuemark.h"
#include"curand_kernel.h"
#include"iostream"
#include <fstream>
#include"const.h"
#include<math.h>
#include"BFS.h"
#include"GAparrel.h"
#include<time.h>
#include <thrust/device_vector.h>
#include <thrust/sort.h>
#include <thrust/functional.h>
#include <thrust/sequence.h>
#include <thrust/copy.h>
#include"PathArrange.h"
int checksum(int*array){
	int sum = 0;
	for (int i = 0; i < Task; i++)
		sum += array[i];
	return sum;
}
__device__ int Curand(unsigned int*seed, unsigned int offset, int*array)
{
	unsigned long m = 31721474647;
	int a = array[(offset*(*seed))% 99999];
	unsigned long x = (unsigned long)*seed;
	x = (a*x) % m;
	*seed = (unsigned int)a*x;
	return((int)a);
}
__global__ void RawChormes(int*chormes,int *pathnum,int*hops,unsigned int*seed,int*array,float*rawvalue,int*rawmark,float*demand){
	int taskid=blockIdx.y;
	int popid=blockIdx.x*blockDim.x+threadIdx.x;
	if(popid>=pop||taskid>=Task)
		return;

	int choice=Curand(seed,taskid,array)%(pathnum[taskid]+1)-1;
	int Cid=popid*Task+taskid;
	chormes[Cid]=choice;
	rawvalue[Cid]=demand[taskid]/pow(hops[taskid*10+choice],0.5);
	rawmark[Cid]=taskid;
}
__global__ void Cook(int*chormes,int*pathset,int pathd,float*popmcap,float*demand,int*rawmark){
	int popid=blockIdx.x*blockDim.x+threadIdx.x;
	if(popid>=pop)
		return;
	for(int i=0;i<Task;i++)
	{
		int mi=rawmark[popid*Task+i];
		int flag=0;
		int k=chormes[popid*Task + mi];
		int dim = mi* 10 * pathd + k*pathd;
		int j=0;
		int e;
		while(true){
			e=pathset[dim+j];
			if(e<0)
				break;
			if(popmcap[popid*EDge+e]<demand[mi])
			{
				flag=1;
				chormes[popid*Task + mi]=-1;
				break;
			}
			j++;
		}
		if(flag==0)
		{
			j=0;
			while(true)
			{
				e=pathset[dim+j];
				if(e<0)
					break;
				popmcap[popid*EDge+e]-=demand[mi];
				j++;
			}
		}
	}
}
/*************************************************
Function:Fitor
Description:caculate objective of all the chromosomes,if the chromosome solution is not feasible(overflow the link capacity)
the objective of the chromosome will be set to a max value.
*************************************************/
__global__ void Fitor(int*hops,float*capacity,int*chormes,float*demand,int *pathset,int pathd,int*fits_key,float*fits_value){
	int chonum = blockIdx.x;
	int threadid = threadIdx.x;
	int blockdim = blockDim.x;
	__shared__ float f[PERB];
	//caculate load;
	for (int i = threadid; i <PERB; i += blockdim)
		f[i] = 0;
	__syncthreads();
	//count the link capacity bandwidth usage, store the value in shared array f.
	for (int i = threadid; i < Task; i += blockdim)
	{
		int k = chormes[chonum*Task + i];
		if (k>=0){
			float deman = demand[i];
			int j = 0;
			int e;
			int dim = i* 10 * pathd + k*pathd;
			while (true){
				e = pathset[dim + j];
				if (e < 0)
					break;
				atomicAdd(&f[e], deman);
				j++;
			}
		}
	}
	__syncthreads(); 
	//judge if some link is overflow and caculate the cost of the link,
	//store the cost in shared array f.
	for (int i = threadid; i <PERB; i += blockdim)
	{
		
		float deman = 0;
		if (i < Task)
		{
			int k = chormes[chonum*Task + i];
			deman = (k<0) ?(INFHOPS*demand[i]):(hops[i*10+k]*demand[i]);
		}
		//if load is overflow ,demand+=100*Task*INFHOPS
		f[i] = (f[i]>capacity[i])?(deman+100*Task*INFHOPS):deman;
	}
	__syncthreads();
	/*reduce add the link cost*/
	if (PERB> (blockdim))
	{
		for (int i = threadid+blockdim; i <PERB; i += blockdim)
			f[threadid] += f[i];
	}
	__syncthreads();
	int size = (PERB<blockdim) ?PERB: blockdim;
	for (int s = size; s>1; s = (s + 1) / 2)
	{
		if (threadid<s/2)
			f[threadid] += f[threadid + (s + 1) / 2];
		__syncthreads();
	}
	if (threadid == 0){
		fits_value[chonum] = f[0];
		fits_key[chonum] =chonum;
	}
	
}
/*************************************************
Function:GetParents
Description:randomly choose parents in parallel
*************************************************/
__global__ void GetParents(int* parents, int*randarray, unsigned int *seed){
	int id = blockIdx.x*blockDim.x+threadIdx.x;
	if (id >= 2 * Beta)
		return;
	if (id < Beta)
		parents[id] = Curand(seed, id, randarray) % ALPHA;
	else
		parents[id] = Curand(seed, id, randarray) % (Beta+ ALPHA);
}
/*************************************************
Function:CudaCross
Description:generate children in parallel
*************************************************/
__global__ void CudaCross(int*children, int*chormes,int*fits_key,int*randarray,unsigned int *seed,int*parents){
	unsigned int blockid = blockIdx.y;
	unsigned int threadid = threadIdx.x + blockIdx.x*blockDim.x;
	if (threadid >= Task)
		return;
	unsigned int position = blockid * 2;
	if (position + 1 >=Beta)
		return;
	int monther = parents[blockid];
	int father = parents[blockid+ Beta];
	int mask = Curand(seed,threadid,randarray) % 2;
	if (mask<1)
	{
		children[position*Task + threadid] = chormes[fits_key[father]*Task + threadid];
		children[(position + 1)*Task + threadid] = chormes[fits_key[monther]*Task + threadid];
	}
	else
	{
		children[position*Task + threadid] = chormes[fits_key[monther]*Task + threadid];
		children[(position + 1)*Task + threadid] = chormes[fits_key[father]*Task + threadid];
	}
	__syncthreads();
}
__global__ void GetMu(int*muinfo,int*children, int*chormes,int*randarray, unsigned int *seed, int*pathnum){
	int id = threadIdx.x + blockIdx.x*blockDim.x;
		if (id >=Gama)
			return;
		int muc = Curand(seed, id, randarray) % pop;
		int mup= Curand(seed, id * 13, randarray) % Task;
		int newv = Curand(seed, id * 71, randarray) % (pathnum[mup] + 1);
		if (newv == pathnum[mup])
			newv = -1;
		muinfo[id * 3] = muc;
		muinfo[id * 3 + 1] = mup;
		muinfo[id * 3 + 2] = newv;
}
__global__ void CudaMutation(int*muinfo,int*children, int*chormes, int*pathnum){
	int conum = blockIdx.y;
	int id = threadIdx.x + blockIdx.x*blockDim.x;
	if (id > Task)
		return;
	if (id == muinfo[3 * conum + 1])
		children[(conum+Beta)*Task + id] = muinfo[3 * conum + 2];
	else
		children[(conum + Beta)*Task + id] =chormes[muinfo[3 * conum] * Task + id];
}
__global__ void Reload(int*chormes,int*children,int*fits_key){
	int conum = blockIdx.y;
	int id = threadIdx.x + blockIdx.x*blockDim.x;
	if (conum>=(Beta+Gama)||id >=Task)
		return;
	chormes[fits_key[conum + ALPHA]*Task + id] = children[conum*Task + id];

}
void NewGAParrel::cudamalloc(){
			cudaMalloc((void**)&dev_chormes, Task*pop*sizeof(int));
			cudaMalloc((void**)&dev_demand, Task*sizeof(float));
			cudaMalloc((void**)&dev_childs, Task*(Beta+Gama)*sizeof(int));
			cudaMalloc((void**)&dev_capacity, G.m*sizeof(float));
			cudaMalloc((void**)&dev_pathset, Task*taskd*sizeof(int));
			cudaMalloc((void**)(&dev_randarray), sizeof(int)*100000);
			cudaMalloc((void**)(&dev_parents), sizeof(int)*Beta*2);
			cudaMalloc((void**)(&dev_seed), sizeof(unsigned int));
			cudaMalloc((void**)(&dev_pathnum), sizeof(int)*Task);
			cudaMalloc((void**)(&dev_muinfo), sizeof(int)*Gama*3);
			cudaMalloc((void**)(&dev_fit_key), sizeof(int)*pop);
			cudaMalloc((void**)(&dev_fit_value), sizeof(float)*pop);
			cudaMalloc((void**)(&dev_hops), sizeof(int)*10*Task);
			cudaMalloc((void**)(&dev_rawvalue), sizeof(float)*pop*Task);
			cudaMalloc((void**)(&dev_rawmark), sizeof(int)*pop*Task);
			cudaMalloc((void**)(&dev_popmcap), sizeof(float)*G.m*pop);
			cudaMemcpy(dev_rawmark,rawmark, sizeof(int)*pop*Task, cudaMemcpyHostToDevice);
			cudaMemcpy(dev_rawvalue,rawvalue, sizeof(float)*pop*Task, cudaMemcpyHostToDevice);
			cudaMemcpy(dev_popmcap,popmcap, sizeof(float)*pop*G.m, cudaMemcpyHostToDevice);
			cudaMemcpy(dev_capacity,capacity, G.m*sizeof(float), cudaMemcpyHostToDevice);
}

void NewGAParrel::cudapre(){
		cudaMemcpy(dev_chormes,chormes, Task*pop*sizeof(int), cudaMemcpyHostToDevice);
		cudaMemcpy(dev_demand,demand,Task*sizeof(float), cudaMemcpyHostToDevice);
		cudaMemcpy(dev_childs,childs,Task*(Beta+Gama)*sizeof(int), cudaMemcpyHostToDevice);
		cudaMemcpy(dev_pathset,pathset,Task*taskd*sizeof(int), cudaMemcpyHostToDevice);
		cudaMemcpy(dev_randarray, randarray, sizeof(int)*100000, cudaMemcpyHostToDevice);
		cudaMemcpy(dev_parents,parents, sizeof(int)*Beta*2, cudaMemcpyHostToDevice);
		cudaMemcpy(dev_seed,seed, sizeof(unsigned int), cudaMemcpyHostToDevice);
		cudaMemcpy(dev_pathnum,pathnum, sizeof(int)*Task, cudaMemcpyHostToDevice);
		cudaMemcpy(dev_muinfo,muinfo, sizeof(int)*Gama*3, cudaMemcpyHostToDevice);
		cudaMemcpy(dev_fit_key,fit_key, sizeof(int)*pop, cudaMemcpyHostToDevice);
		cudaMemcpy(dev_fit_value,fit_value, sizeof(float)*pop, cudaMemcpyHostToDevice);
		cudaMemcpy(dev_hops,hops, sizeof(int)*10*Task, cudaMemcpyHostToDevice);
	}
void NewGAParrel::cudafree(){
		cudaFree(dev_chormes);
		cudaFree(dev_demand);
		cudaFree(dev_childs);
		cudaFree(dev_capacity);
		cudaFree(dev_pathset);
		cudaFree(dev_randarray);
		cudaFree(dev_parents);
		cudaFree(dev_seed);
		cudaFree(dev_pathnum);
		cudaFree(dev_muinfo);
		cudaFree(dev_fit_key);
		cudaFree(dev_fit_value);
		cudaFree(dev_hops);
		cudaFree(dev_rawmark);
		cudaFree(dev_rawvalue);
		cudaFree(dev_popmcap);
	}
void NewGAParrel::parrelmake(){
	dim3 blocks_s(pop/512 + 1,Task);
	RawChormes<< <blocks_s,512>> >(dev_chormes,dev_pathnum,dev_hops,dev_seed,dev_randarray,dev_rawvalue,dev_rawmark,dev_demand);
	thrust::device_ptr<float> dev_rv(dev_rawvalue);
	thrust::device_ptr<int> dev_rm(dev_rawmark);
		for(int i=0;i<pop;i++)
			thrust::sort_by_key((dev_rv+i*Task),(dev_rv+(i+1)*Task) ,(dev_rm+i*Task),thrust::greater<float>());
	cudaMemcpy(rawmark,dev_rawmark, sizeof(int)*pop*Task, cudaMemcpyDeviceToHost);
	Cook<< <pop,512>> >(dev_chormes,dev_pathset,pathd,dev_popmcap,dev_demand,dev_rawmark);
}
void NewGAParrel::process(){

}

/*************************************************
Function:GAsearch
Description:parallel GA search realization
*************************************************/
vector<pair<string,float> > NewGAParrel::GAsearch(){
	cudamalloc();
	cout<<"GA Parallel searching......."<<endl;
	float start=float(1000*clock())/ CLOCKS_PER_SEC;
	/*initialize chromosomes*/
	GoldnessMake();
	/*copy data to GPU memory*/
	cudapre();
	best =Task*100*INFHOPS;/*best objective record*/
	int count = 0;
	int iter=0;
	thrust::device_ptr<float> dev_fv(dev_fit_value);
	thrust::device_ptr<int> dev_fk(dev_fit_key);
	vector<float>middata;
	int mkd=2;
	for (int i = 0; i <10000000; i++)
	{
		iter++;
		seed++;
		Fitor << <pop,1024 >> >(dev_hops,dev_capacity, dev_chormes, dev_demand, dev_pathset, pathd, dev_fit_key,dev_fit_value);
		/*sort the chromosomes by objective value*/
		thrust::sort_by_key(dev_fv,dev_fv+pop ,dev_fk,thrust::less<float>());
		cudaMemcpy(fit_value,dev_fit_value, sizeof(float)*pop, cudaMemcpyDeviceToHost);
		float ans=fit_value[0];
		/*randomly choose parents*/
		GetParents << <(Beta * 2 + 511) / 512, 512 >> >(dev_parents,dev_randarray,dev_seed);
		dim3 blocks_s(Task / 1024 + 1, Beta / 2 + 1);
		/*cross parents chromosomes to get children*/
		CudaCross << <blocks_s, 1024 >> >(dev_childs,dev_chormes,dev_fit_key,dev_randarray,dev_seed,dev_parents);
		GetMu << <Gama / 1024 + 1, 1024 >> >(dev_muinfo,dev_childs,dev_chormes,dev_randarray,dev_seed,dev_pathnum);
		dim3 blocks_s2(Task / 1024 + 1, Gama);
		/*randomly mutate chromosomes*/
		CudaMutation << <blocks_s2, 1024 >> >(dev_muinfo, dev_childs, dev_chormes, dev_pathnum);
		dim3 blocks_s3(Task / 1024 + 1, Gama + Beta);
		Reload << <blocks_s3, 1024 >> >(dev_chormes, dev_childs, dev_fit_key);
		if (ans<best)
		{
			mkd--;
			best = ans;
			count = 0;
		}
		else
			count++;
		middata.push_back(ans);
		if(mkd>0&&count<100)
			continue;
		time_t now=1000*clock()/ CLOCKS_PER_SEC;
		if (count>loomore||((now-start)>EXPIRE&&GANOEX<0))
			break;
	}
	cudaMemcpy(chormes,dev_chormes, Task*pop*sizeof(int), cudaMemcpyDeviceToHost);
	cudaMemcpy(fit_key,dev_fit_key, sizeof(int)*pop, cudaMemcpyDeviceToHost);
	float end = float(1000*clock())/ CLOCKS_PER_SEC;
	pair<float,int>md=more();
	vector<pair<string,float>>rdata;
	float lowbound=0;
	for(int i=0;i<Task;i++)
		lowbound+=demand[i]*INFHOPS;
	float gap=middata[middata.size()-1]-best;
	cout<<"gap is"<<gap<<endl;
	for(int i=0;i<middata.size();i++)
		middata[i]-=gap;
	CheckR(&G,Result,serv,string("GA_Paralle"));
	writejsoniter(GAPFILE,middata,string("GA_Paralle"));
	rdata.push_back(make_pair(string("object"),best));
	rdata.push_back(make_pair(string("inf_obj"),lowbound));
	rdata.push_back(make_pair(string("task_add_in"),md.second));
	rdata.push_back(make_pair(string("flow_add_in"),md.first));
	rdata.push_back(make_pair(string("total_weight"),totalweight));
	rdata.push_back(make_pair(string("time"),(end-start)+affier));
	rdata.push_back(make_pair(string("iter_num"),iter));
	rdata.push_back(make_pair(string("iter_time"),float(end-start+affier)/iter));
	rdata.push_back(make_pair(string("gap"),gap));
	writejsondata(DATAFILE,rdata,string("GA_Paralle"));
	return rdata;
}


