#include <iostream>
#include "HashTable.hpp"
#include <unordered_set>
#include <sstream>
#include <vector>

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

unordered_set<string> readNodes (int numericGraphStructure, string filePath)
{
    // List all nodes (HashSet, so there are no duplicates)
    unordered_set<string> nodes;

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
                for (int i = 0; i < nodeElements.size(); i++)
                {
                    // Iterating over each node found
                    nodes.insert(nodeElements[i]);
                }
            }
            break;
    }

    return nodes;
}

void writeAnonymizedGraph (HashTable<string, string> permutationFunction, int numericGraphStructure, string filePath)
{
    ifstream file(filePath);
    string anonymizedGraphFileName = "anonymizedGraph.txt";

    ofstream newGraphFile    (anonymizedGraphFileName,std::ofstream::binary);
    switch (numericGraphStructure)
    {
        case 1:
            string str;
            while (getline(file, str))
            {
                // Each line is an edge - so there are two nodes per line, separated by whitespace
                vector<string> nodeElements;
                split(str, ' ', std::back_inserter(nodeElements));
                // There are only two nodes - So nodeElements.size() == 2
                newGraphFile << permutationFunction.get(nodeElements[0]) + " " + permutationFunction.get(nodeElements[1]) << endl;
            }
            break;
    }
}

int main (int argc, char * argv[])
{
    // Input: graph (G)
    // Path to graph G and way it`s structured
    if(argc != 3)
    {
        cout << "Wrong number of parameters." << endl;
        return EXIT_FAILURE;
    }

    string filePath(argv[1]);

    string graphStructure(argv[2]);
    int numericGraphStructure = getGraphStructure(graphStructure);

    // Reading input file
    unordered_set<string> nodes = readNodes(numericGraphStructure, filePath);

    // Run the random permutation
    vector<string> vectorNodes; // we need it to be a vector, since we want to run the std permutation function
    for (const std::string& x: nodes) vectorNodes.push_back(x);
    random_shuffle (vectorNodes.begin(), vectorNodes.end());

    // Permutation function
    int counter = 0;
    HashTable<string, string> permutationFunction;
    for (auto it = nodes.begin(); it != nodes.end(); ++it)
    {
        // *it was transformed to vectorNodes[counter]
        permutationFunction.set(*it, vectorNodes[counter]);
        counter++;
    }

    // Write new graph and permutation function
    writeAnonymizedGraph(permutationFunction, numericGraphStructure, filePath);
    permutationFunction.writeToFile("permutationFunction.txt");

    // Output: anonymized graph (AG) and function used for anonymization
    // ex: node a in G is node 121 in AG

    return 0;
}