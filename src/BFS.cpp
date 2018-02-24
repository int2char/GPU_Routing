#include"Graph.h"
#include"dijkstra.h"
#include"Heap.h"
#include<queue>
#include"LinkQueue.h"
#define INF 100000
#define N 10000
int flag2[N];
void BFS(Graph *G, int s, int t, float d[], int peg[], float demand, float capacity[]){
	int n_num = G->n;
	for (int i = 0; i < n_num; i++)
		if (i == s)
			d[i] = 0;
		else
			d[i] = INF;
	for (int i = 0; i < n_num; i++)
	{
		flag2[i] = 0;
		peg[i] = -1;
	}
	int cur = s;
	LinkQueue que;
	que.push(s);
	do{
		int cur = que.pop();
		flag2[cur] = 1;
		if (cur == t)
			break;
		int size = G->near[cur].size();
		for (int i = size-1; i>=0; i--)
		{ 
			int id = G->near[cur][i];
			if (capacity[id] >= demand)
			{
				int head = G->incL[id].head;
				int tail = G->incL[id].tail;
				if (flag2[head] == 0 && d[head] > d[tail] + 1)
				{
					d[head] = d[tail] + 1;
					peg[head] = id;
					que.push(head);
				}
			}
		}
	} while (!que.isEmpty());
}
