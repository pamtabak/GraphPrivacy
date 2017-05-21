#!/bin/bash

#sudo chmod 755 filename

number_of_iterations=50

targeted_nodes_list="600 700 800 900 1100 1200 1300 1400 1600 1700 1800 1900 2100 2200 2300 2400 2600 2700 2800 2900 3100 3200 3300 3400 3600 3700 3800 3900 4039"

for target in $targeted_nodes_list;
do
	echo "$target"
	for ((i=0; i < $number_of_iterations; ++i ))
	do	
		targeted_nodes=`python generate_targeted_nodes.py $target`
		
		current_dir=$(pwd)
		output_folder="$current_dir/Files/"
		graph="$current_dir/Files/facebook_combined.txt"

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

		rm -rf "$current_dir/Files/degree.txt"
		rm -rf "$current_dir/Files/subgraph.txt"
		rm -rf "$targeted_nodes"
		rm -rf "$full_graph"
		rm -rf "$current_dir/Files/permutationFunction.txt"
	done
done