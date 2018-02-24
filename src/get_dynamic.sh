#!/bin/bash
rm -f ./data/*.txt
nodes=(50 100 200 300 500 800 1000 2000)
types=(0 2)
graphtype=("ER" "NI" "BA")
powers=(1 2 3 4 5 6)
capacitys=(150 200 250)
declare -i edge task biao
for node in ${nodes[@]}
do

  for type in ${types[@]}
  do
   path="../dynamic_G/$node/$type/"
   edge=`cat $path/Graph.txt|wc -l`
   edge=$edge
   task=$[3*$node]
   biao=$type
   rm -f *.out
   nvcc -O3 -std=c++11 *.cpp *.cu --gpu-architecture=compute_35 --gpu-code=sm_35 -I ../include -I ../cplex_include -L ../lib -lconcert -lcplex -lilocplex -lm -lpthread -DIL_STD -DNODE=$node -DEDge=$edge -DTask=$task -DTYPE="\"${graphtype[$biao]}\"" -DINPUTFILE=\"$path\" -DGANOEX=1 -DGRAPHTYPE=-1 #2>>./data/complie.txt
   CUDA_VISIBLE_DEVICES=0 ./a.out L #1>>./data/runinfo.txt 2>>./data/err.txt
  done
done

