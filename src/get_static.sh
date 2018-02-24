#!/bin/bash
#clean the result folder
rm -f ./data/*.txt
nodes=(50) #200 300 500 800 1000 2000)
types=(0) #2)
graphtype=("ER" "NI" "BA")
powers=(6)
capacitys=(150 200 250)
declare -i edge task biao
# ergodic all kinds of size of network topo
for node in ${nodes[@]}
do
  #ergodic two kinds of Graph type(ER or BA)
  for type in ${types[@]}
  do
   path="../static_G/$node/$type/"
   edge=`cat $path/Graph.txt|wc -l`
   edge=$edge+$edge
   #ergodic all kinds of service size
   for power in ${powers[@]}
    do
      task=$[$power*$node]
      biao=$type
      rm -f *.out
      #complie command 
      nvcc -O3 -std=c++11 *.cpp *.cu --gpu-architecture=compute_35 --gpu-code=sm_35 -I ../include -I ../cplex_include -L ../lib -lconcert -lcplex -lilocplex -lm -lpthread -DIL_STD -DNODE=$node -DEDge=$edge -DTask=$task -DTYPE="\"${graphtype[$biao]}\"" -DINPUTFILE=\"$path\" -DGANOEX=1 #2>>./data/complie.txt
     #run the  LR-SROA method
      CUDA_VISIBLE_DEVICES=0 ./a.out L #1>>./data/runinfo.txt 2>>./data/err.txt
      if [[ $power -eq 6 ]];then
      #vary the link capacity size
      for capacity in ${capacitys[@]}
       do
          rm -f *.out
          nvcc -O3 -std=c++11 *.cpp *.cu --gpu-architecture=compute_35 --gpu-code=sm_35 -I ../include -I ../cplex_include -L ../lib -lconcert -lcplex -lilocplex -lm -lpthread -DIL_STD -DNODE=$node -DEDge=$edge -DTask=$task -DTYPE="\"${graphtype[$biao]}\"" -DINPUTFILE=\"$path\" -DGANOEX=1 -DCAPACITY=$capacity #2>>./data/complie.txt
    CUDA_VISIBLE_DEVICES=0 ./a.out L #1>>./data/info.txt 2>>./data/err.txt
        done
      fi
    done
  done
done
