#include"Edge.h"
#define N 10000
class Heap
{
public:
	Heap();
	~Heap();
	void push(int vertID, int w);
	void update(int vertID, int w);
	int pop();
	int empty();
private:
	Edge *h[N + 10];
	int post[N + 10];
	int nodeNum;
	void fix(int fixID);

};


