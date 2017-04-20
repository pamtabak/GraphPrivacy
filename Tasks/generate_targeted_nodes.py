import sys
import os

number_of_nodes = int(sys.argv[1])

filename = "Files/targeted_nodes" + str(number_of_nodes) + ".txt"
target = open(filename, 'w')

# write it fo file
for i in xrange(0, number_of_nodes):
	target.write(str(i) + "\n")

target.close()
print(os.path.abspath(filename))