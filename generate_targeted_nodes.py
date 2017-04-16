import sys

number_of_nodes = int(sys.argv[1])

# write it fo file
target = open("targeted_nodes.txt", 'w')
for i in xrange(1, number_of_nodes + 1):
	target.write(str(i) + "\n")

target.close()