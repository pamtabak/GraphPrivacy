#!/bin/bash

#sudo chmod 755 filename

graph=`python generate_gnp.py 1000 0.1`
#echo $graph #filename, inside Tasks/Files/. This is the graph G

targeted_nodes=`python generate_targeted_nodes.py 100`

current_dir=$(pwd)
output_folder="$current_dir/Files/"

#create attackers
cd ..
cd CreateAttackers/cmake-build-debug
./CreateAttackers "$graph" "$targeted_nodes" "$output_folder" edgesPerLine

#create new file appending the subgraph generated to the graph G
cd ..
cd ..
cd Tasks
current_dir=$(pwd)
full_graph=`python append_files.py "$graph" "$current_dir/Files/subgraph.txt"`

#anonimize
cd ..
cd RandomPermutation/cmake-build-debug
./RandomPermutation "$full_graph" "$output_folder" edgesPerLine

cd ..
cd ..

#recover graph
cd RecoverGraph/cmake-build-debug
./RecoverGraph "$full_graph" "$current_dir/Files/degree.txt" "$output_folder" edgesPerLine "$current_dir/Files/subgraph.txt"