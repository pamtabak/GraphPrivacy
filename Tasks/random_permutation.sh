#!/bin/bash

#sudo chmod 755 random_permutation

current_dir=$(pwd)
output_folder="$current_dir/Files/"
graph="$current_dir/Files/teste5nos.txt"

cd ..
cd RandomPermutation/cmake-build-debug
./RandomPermutation "$graph" "$output_folder" edgesPerLine NOTrandomIdentifiers