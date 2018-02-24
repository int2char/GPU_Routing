#include<stdlib.h>
#include "LinkQueue.h"
#include"node.h"

LinkQueue::LinkQueue()
{
	size = 0;
}
LinkQueue::~LinkQueue()
{
}
void LinkQueue::push(int val)
{
		node*p =(node*)malloc(sizeof(node));
		p->val = val;
		p->next = NULL;
		if (size == 0)
		{
			head = p;
			tail = p;
			size += 1;
		}
		else
		{
			size += 1;
			tail->next = p;
			tail = p;
		}

}
int LinkQueue::pop()
{
	if (size == 0)
		return-1;
	if (size > 1)
	{
		int val = head->val;
		node* np = head->next;
		delete head;
		head = np;
		size--;
		return val;
		
	}
	else
	{
		int val = head->val;
		delete head;
		head = NULL;
		size--;
		return val;
	}

}
bool LinkQueue::isEmpty(){
	if (size == 0)
		return true;
	else
		return false;
}
