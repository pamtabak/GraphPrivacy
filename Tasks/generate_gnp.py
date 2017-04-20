import sys
import random
import os

n = int(sys.argv[1])
p = float(sys.argv[2])

# write it fo file
filename = "Files/model_gnp_" + str(n) + "_" + str(p).replace(".",",") + ".txt"
target = open(filename, 'w')

for i in range (0, n):
	for j in range (i + 1, n):
		#Return the next random floating point number in the range [0.0, 1.0).
		if (random.random() < p):
			target.write(str(i) + " " + str(j) + "\n")

target.close()
print(os.path.abspath(filename))