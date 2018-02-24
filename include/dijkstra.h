#include"Graph.h"
#include <iostream>
using namespace std;
void dijkstra(Graph *G, int s, int t, float d[], int peg[], float lambda[]);
void dijcapacity(Graph *G, int s, int t, float d[], int peg[], float lambda[],float capacity[],float demand);
