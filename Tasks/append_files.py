import sys
import os

filenames = [sys.argv[1], sys.argv[2]]
filename  = 'Files/full_graph.txt'
with open(filename, 'w') as outfile:
    for fname in filenames:
        with open(fname) as infile:
            for line in infile:
                outfile.write(line)

outfile.close();
print(os.path.abspath(filename))