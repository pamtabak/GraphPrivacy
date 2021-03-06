from graph_tool.all import *
import math
import sys
import random
import os

n = int(sys.argv[1]) # number of nodes
m = int(sys.argv[2]) # initial degree of each node (also minimum possible degree)

g = price_network(n,m,directed=False)

edges = {}

filename = "Files/model_ba_" + str(n) + "_" + str(m) + ".txt"
target = open(filename, 'w')

for v in g.vertices():
	v_label = str(g.vertex_index[v])
	for w in v.out_neighbours():
		# avoiding saving the same edge twice (since it's an undirected graph)
		w_label = str(g.vertex_index[w])
		full_label_1 = v_label + " " + w_label
		full_label_2 = w_label + " " + v_label
		if (full_label_1 not in edges and full_label_2 not in edges):
			edges[full_label_1] = 0
			edges[full_label_2] = 0
			target.write(full_label_1 + "\n")

target.close()
print(os.path.abspath(filename))