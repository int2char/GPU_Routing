#include"node.h"
#pragma once
class LinkQueue
{
private:
	node* head;
	node* tail;
	int size;
public:
	LinkQueue();
	void push(int va);
	int pop();
	bool isEmpty();
	~LinkQueue();
};

