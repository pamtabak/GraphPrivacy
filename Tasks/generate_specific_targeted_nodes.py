import sys
import os

number_of_nodes = int(sys.argv[1])

nodes = []
input_filename = "Files/WikiVoteNodes.txt"
with open(input_filename) as infile:
	for line in infile:
		nodes.append(line.replace("\n",""))

filename = "Files/targeted_nodes" + str(number_of_nodes) + ".txt"
target = open(filename, 'w')

# write it fo file
for i in xrange(0, number_of_nodes):
	target.write(nodes[i] + "\n")

target.close()
print(os.path.abspath(filename))