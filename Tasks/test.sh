#!/bin/bash

#sudo chmod 755 filename

graph=`python generate_gnp.py 100 0.1`
#echo $graph #filename, inside Tasks/Files/. This is the graph G

targeted_nodes=`python generate_targeted_nodes.py 10`

current_dir=$(pwd)
output_folder="$current_dir/Files/"

#create attackers
cd ..
cd CreateAttackers/cmake-build-debug
./CreateAttackers "$graph" "$targeted_nodes" "$output_folder" edgesPerLine

#create new file appending the subgraph generated to the graph G

#anonimize

#recover graph