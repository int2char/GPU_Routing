#ifndef __EDGE__
#define __EDGE__

struct Edge
{
public:
	int head, tail;
    float weight;
	float capacity;
	float backweight;
	Edge(int s,int t,int w,int c,int w2):head(t),tail(s),weight(w),capacity(c),backweight(w2){};
private:

};

#endif
