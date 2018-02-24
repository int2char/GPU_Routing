/*************************************************
Copyright:UESTC
Author: ZhangQian
Date:2016-08-25
Description:define some constant parameters.
**************************************************/
#ifndef CONSTH
	#define CONSTH
	#ifndef NODE
		#define NODE 50
	#endif
	#ifndef Task
		#define Task 300
	#endif
	#ifndef EDge
		#define EDge 304
	#endif
	#ifndef CAPACITY
		#define CAPACITY 100
	#endif
	#ifndef TYPE
		#define TYPE "BA"
	#endif
	#ifndef GRAPHTYPE
		#define GRAPHTYPE 1
	#endif
	#ifndef GANOEX
		#define GANOEX -1
	#endif
	#ifndef INPUTFILE
		#define INPUTFILE "../result/100/0/"
	#endif
	#ifndef EXPIRE
		#define EXPIRE 600000
	#endif
	#define PERB ((Task>EDge)?Task:EDge)
	#define INFHOPS 10
	#define Deep 300
	#define loop 30
	#define loomore 30
	#define FROM 0
	#define STATMAX 100*pop
	#define MAXITER 1000000

	#define LAGPFILE "./data/iteration.txt"
	#define LAGSFILE "./data/iteration.txt"
	#define GAPFILE "./data/iteration.txt"
	#define GASFILE "./data/iteration.txt"

	#define ANSWERS "./data/answers.txt"
    #define TOPFILE "Graph.txt"
	#define ROUTFILE "data.txt"
	#define FLOWFILE "service.txt"

	#define INFOFILE "./data/info.txt"
	#define DATAFILE "./data/all_data.txt"
	#define FLOWVARRY "./data/flow_varry.txt"
	#define EDGE 1500
	#define pop 5000
	#define ALPHA 500
	#define Beta 1000
	#define Gama 3500
#endif
