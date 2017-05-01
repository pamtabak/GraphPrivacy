#!/bin/bash

#sudo chmod 755 filename

number_of_iterations=50

targeted_nodes_list="10"

for target in $targeted_nodes_list;
do
	echo "$target"
	for ((i=0; i < $number_of_iterations; ++i ))
	do	
		targeted_nodes=`python generate_targeted_nodes.py $target`
		
		current_dir=$(pwd)
		output_folder="$current_dir/Files/"
		graph="$current_dir/Files/test1.txt"

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
	done
done