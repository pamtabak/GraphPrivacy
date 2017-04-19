import sys
import random

# write it fo file
target = open("model_gnp.txt", 'w')

n = int(sys.argv[1])
p = float(sys.argv[2])

for i in range (0, n):
	for j in range (i + 1, n):
		#Return the next random floating point number in the range [0.0, 1.0).
		if (random.random() < p):
			target.write(str(i) + " " + str(j) + "\n")

target.close()