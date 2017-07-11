import sys
import os

filename  = 'Files/gnp_attack_2.txt'
with open(filename, 'w') as outfile:
	with open('Files/full_graph.txt') as infile:
		for line in infile:
			outfile.write(line.replace("\n","") + " Undirected\n")

outfile.close();