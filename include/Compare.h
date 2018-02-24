#ifndef __COMPARE__

#define __COMPARE__

#include "Edge.h"
#include"Graph.h"
#include"service.h"
#include<vector>
using namespace std;

class Compare{
private:
	void  bellmanFord(Graph *G, int st, int te, int* p,int* nullmap);
	static int cmp(service s1, service s2);
public:
	//Compare();
	int Rough(Graph* G, vector<service> s);


};
#endif
