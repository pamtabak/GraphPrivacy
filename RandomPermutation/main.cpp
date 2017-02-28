#include <iostream>
#include "HashTable.hpp"
#include <unordered_set>
#include <sstream>
#include <vector>
#include <algorithm>

using namespace std;

HashTable<string, int> generatePossibleGraphStructures ()
{
    HashTable<string, int> possibleGraphStructures;
    possibleGraphStructures.set("edgesPerLine", 1);

    return possibleGraphStructures;
}

int getGraphStructure (string graphStructure)
{
    HashTable<string, int> possibleGraphStructures = generatePossibleGraphStructures();

    return possibleGraphStructures.get(graphStructure);
}

template<typename Out>
void split(const std::string &s, char delimiter, Out result) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delimiter)) {
        *(result++) = item;
    }
}

HashTable<string, unordered_set<string>> readGraph (int numericGraphStructure, string filePath, unordered_set<string> &nodes)
{
    HashTable<string, unordered_set<string>> graph;

    ifstream file(filePath);
    switch (numericGraphStructure)
    {
        case 1:
            string str;
            while (getline(file, str))
            {
                // Each line is an edge - so there are two nodes per line, separated by whitespace
                vector<string> nodeElements;
                split(str, ' ', std::back_inserter(nodeElements));
                unordered_set<string> neighbors = graph.get(nodeElements[0]);
                neighbors.insert(nodeElements[1]);
                graph.set(nodeElements[0], neighbors);

                // Listing all different nodes
                nodes.insert(nodeElements[0]);
                nodes.insert(nodeElements[1]);
            }
            break;
    }

    return graph;
}

HashTable<string, string> randomPermutation (unordered_set<string> nodes, bool randomIdentifiers)
{
    // Since the std permutation function takes a vector as a param, we need to transform our nodes data into a vector
    vector<string> vectorNodes;
    for (const string& x: nodes)
    {
        vectorNodes.push_back(x);
    }
    random_shuffle (vectorNodes.begin(), vectorNodes.end());

    HashTable<string, string> permutationFunction;

    if (randomIdentifiers)
    {
        /* Each node`s label becomes a numeric  id
         * In this case, it`s index on the permuted vector */
        for (int i = 0; i < vectorNodes.size(); i++)
        {
            permutationFunction.set(vectorNodes[i], to_string(i));
        }
    }
    else
    {
        /* Node`s labels are replaced between themselves */
        int counter = 0;
        for (auto it = nodes.begin(); it != nodes.end(); ++it)
        {
            // *it was transformed to vectorNodes[counter]
            permutationFunction.set(*it, vectorNodes[counter]);
            counter++;
        }
    }

    return permutationFunction;
}

void writeAnonymizedGraph (HashTable<string, string> permutationFunction, HashTable<string, unordered_set<string>> graph, string outputDirectoryPath)
{
    ofstream newGraphFile (outputDirectoryPath + "anonymizedGraph.txt", ofstream::binary);
    unordered_map<string, unordered_set<string>> map = graph.getMap();
    for (unsigned i = 0; i < map.bucket_count(); ++i)
    {
        for (auto local_it = map.begin(i); local_it!= map.end(i); ++local_it)
        {
            unordered_set<string> neighbors = local_it->second;
            for (const string& x: neighbors)
            {
                newGraphFile << permutationFunction.get(local_it->first) + " " + permutationFunction.get(x) << endl;
            }
        }
    }
}

int main (int argc, char * argv[])
{
    /* If true, each node`s label becomes a numeric  id
     * If false, the node`s labels are replaced between themselves */
    bool randomIdentifiers = true;
    if (argc < 4 || argc > 5)
    {
        cout << "Wrong number of parameters." << endl;
        return EXIT_FAILURE;
    }
    else if (argc == 5)
    {
        if (argv[4] != "randomIdentifiers")
            randomIdentifiers = false;
    }

    string filePath(argv[1]);
    string outputDirectoryPath(argv[2]);
    string graphStructure(argv[3]);
    int numericGraphStructure = getGraphStructure(graphStructure);

    unordered_set<string> nodes;
    HashTable<string, unordered_set<string>> graph = readGraph(numericGraphStructure, filePath, nodes);

    HashTable<string, string> permutationFunction = randomPermutation(nodes, randomIdentifiers);

    // Write new graph and permutation function
    writeAnonymizedGraph(permutationFunction, graph, outputDirectoryPath);
    permutationFunction.writeToFile(outputDirectoryPath + "permutationFunction.txt");

    // Output: anonymized graph (AG) and function used for anonymization
    // ex: node a in G is node 121 in AG

    return 0;
}