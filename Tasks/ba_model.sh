#!/bin/bash

#sudo chmod 755 filename
number_of_nodes=1000
number_of_iterations=50
echo $number_of_nodes

targeted_nodes_list="2 4 8 16 32 64 100 128 200 256 300 400 500 512 600 700 800 900 1000"
m_list="5 10 20 30 40 50 100"

for target in $targeted_nodes_list;
do
	for m in $m_list;
	do
		echo "$target"
		echo "$m"
		for ((i=0; i < $number_of_iterations; ++i ))
		do
			graph=`python generate_ba_model.py $number_of_nodes $m`
			
			targeted_nodes=`python generate_targeted_nodes.py $target`
			
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
			./RecoverGraph "$full_graph" "$current_dir/Files/degree.txt" "$output_folder" edgesPerLine "$current_dir/Files/subgraph.txt" "$current_dir/Files/permutationFunction.txt"
		
			cd ..
			cd ..
			cd Tasks

			rm -rf "$graph"
			rm -rf "$current_dir/Files/degree.txt"
			rm -rf "$current_dir/Files/subgraph.txt"
			rm -rf "$full_graph"
			rm -rf "$targeted_nodes"
			rm -rf "$current_dir/Files/permutationFunction.txt"
		done
	done
done