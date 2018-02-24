#include"Heap.h"
#include<iostream>
using namespace std;
void Heap::fix(int fixID){
	while (fixID > 1){
		int fat = fixID / 2;
		if (h[fixID]->weight< h[fat]->weight){
			swap(post[h[fixID]->head], post[h[fat]->head]);
			swap(h[fixID], h[fat]);
			fixID /= 2;
		}
		else
			break;
	}
}
Heap::Heap(){
	nodeNum = 0;
}
void Heap::push(int vertID, int w){
	nodeNum++;
	h[nodeNum] = (Edge *)malloc(sizeof(Edge));
	h[nodeNum]->head = vertID;
	h[nodeNum]->weight = w;
	post[vertID] = nodeNum;
	fix(nodeNum);
}
void Heap::update(int vertID, int w){
	int p = post[vertID];
	h[p]->weight = w;
	fix(p);
}
int Heap::pop(){
	int ret = h[1]->head;
	free(h[1]);
	h[1] = h[nodeNum--];
	if (nodeNum > 0)
		post[h[1]->head] = 1;

	int cur = 1;
	while (cur * 2 <= nodeNum){
		int son = cur * 2;
		if (son + 1 <= nodeNum && h[son]->weight> h[son + 1]->weight)
			son = son + 1;
		if (h[cur]->weight > h[son]->weight){
			swap(post[h[cur]->head], post[h[son]->head]);
			swap(h[cur], h[son]);
			cur = son;
		}
		else
			break;
	}

	return ret;
}
int Heap::empty(){
	if (nodeNum <= 0)
		return 1;
	else
		return 0;
}
Heap::~Heap(){
	while (nodeNum > 0)
		pop();
}