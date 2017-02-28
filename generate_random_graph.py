import sys
import random

# This must be a connected graph

number_of_nodes = int(sys.argv[1])

# create number_of_nodes
nodes = {}
nodes_list = []
for i in xrange(0, number_of_nodes):
	nodes[i] = []
	nodes_list.append(i)

# create edges
for node in nodes_list:
	# get 3 random nodes and connect them
	for i in xrange(0, 3):
		number = random.choice(nodes_list)
		if (number != node and number not in nodes[node] and node not in nodes[number]):
			nodes[node].append(number)
	while (len(nodes[node]) == 0):
		# it must be a connected graph
		number = random.choice(nodes_list)
		if (number != node and number not in nodes[node] and node not in nodes[number]):
			nodes[node].append(number)

# write it fo file
target = open("graph.txt", 'w')
for node in nodes:
	for x in nodes[node]:
		target.write(str(node) + " " + str(x) + "\n")

target.close()