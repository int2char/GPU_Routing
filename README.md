# GPU_Routing
source code of paper "Toward Efficient Parallel Routing Optimization for Large-scale SDN Networks Using GPGPU"
##  Dependencies
    1.c++11
    2.cuda8.0
##  File Struct
    cpex_include:cplex include file.
    lib:cplex library.
    include&src:LR-PROA,LR-SROA,GA-PROA,GA-SROA source code.
    static_G:Graph topo files folder
    interpretation:
      static_G/50/0 means the ER graph with 50 node.
      static_G/100/2 means the BA graph with 100 node.
      static_G/50/0/data.txt:cadidate routes
      static_G/50/0/Graph.txt:link start and end node,link capacity.
      static_G/50/0/service.txt:service start and destination,service bandwidth demand.
    dynamic_G:Graph of a dynamic network in which some link has been occupied.
##  Complie and run
    Complie parameter
    -D NODE:Graph node number
       Edge:Graph edge number
       Task:Service number
       TYPE:BA:2,ER:0
       INPUTFILE:data input file directory path
       CAPACITY:link capacity(=100 default)
       GRAPHTYPE:static:1,dynamic:-1
    run parameter
        L:run LR-PROA
        S:run LR-SROA
        G:run GA-PROA
        T:run GA-SROA
        J:just check the topo files
example:
the bash code blow will generate the result of LR-SROA method for node number vary from 50 to 2000 for both kinds of Graph type. 
```bash
     """
     example bash code get_dynamic.sh
     """
        #clean the result folder
        rm -f ./result/*.txt
        nodes=(50 100 200 300 500 800 1000 2000)
        types=(0 2)
        graphtype=("ER" "NI" "BA")
        powers=(1 2 3 4 5 6)
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
              nvcc -O3 -std=c++11 *.cpp *.cu --gpu-architecture=compute_35 --gpu-code=sm_35 -I ../include -I ../cplex_include -L ../lib -lconcert -lcplex -lilocplex -lm -lpthread -DIL_STD -DNODE=$node -DEDge=$edge -DTask=$task -DTYPE="\"${graphtype[$biao]}\"" -DINPUTFILE=\"$path\" -DGANOEX=1 2>>./data/complie.txt
             #run the  LR-SROA method
              CUDA_VISIBLE_DEVICES=0 ./a.out L 1>>./data/runinfo.txt 2>>./data/err.txt
              if [[ $power -eq 6 ]];then
              #vary the link capacity size
              for capacity in ${capacitys[@]}
               do
                  rm -f *.out
                  nvcc -O3 -std=c++11 *.cpp *.cu --gpu-architecture=compute_35 --gpu-code=sm_35 -I ../include -I ../cplex_include -L ../lib -lconcert -lcplex -lilocplex -lm -lpthread -DIL_STD -DNODE=$node -DEDge=$edge -DTask=$task -DTYPE="\"${graphtype[$biao]}\"" -DINPUTFILE=\"$path\" -DGANOEX=1 -DCAPACITY=$capacity 2>>./data/complie.txt
            CUDA_VISIBLE_DEVICES=0 ./a.out S 1>>./data/info.txt 2>>./data/err.txt
                done
              fi
            done
          done
        done
```
##  program result 
    1.the time and objective result will be written into src/data/all_data.txt in a json type.
    2.middle iteration result will be written into /src/data/iteration.txt in a json type.

        
        
       
       
       
       
