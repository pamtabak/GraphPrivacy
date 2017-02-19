#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

unordered_map<string, int> generatePossibleGraphStructures ()
{
    unordered_map<string, int> possibleGraphStructures;
    possibleGraphStructures["edgesPerLine"] =  1;

    return possibleGraphStructures;
}

int getGraphStructure (string graphStructure)
{
    unordered_map<string, int> possibleGraphStructures = generatePossibleGraphStructures();

    return possibleGraphStructures[graphStructure];
}

template<typename Out>
void split(const std::string &s, char delimiter, Out result) {
    stringstream ss;
    ss.str(s);
    string item;
    while (getline(ss, item, delimiter)) {
        *(result++) = item;
    }
}

unordered_map<string, unordered_set<string>> readGraph (int numericGraphStructure, string filePath)
{
    unordered_map<string, unordered_set<string>> graph;

    ifstream file(filePath);
    switch (numericGraphStructure)
    {
        case 1:
            string str;
            while (getline(file, str))
            {
                // Each line is an edge - so there are two nodes per line, separated by whitespace
                vector<string> nodeElements;
                split(str, ' ', back_inserter(nodeElements));
                graph[nodeElements[0]].insert(nodeElements[1]);
                graph[nodeElements[1]].insert(nodeElements[0]);
            }
            break;
    }
}

void getAttackersDegree(string filePath, int *attackersDegree[])
{
    ifstream file(filePath);
    string str;
    int counter = 0;
    int numberOfAttackers;
    while (getline(file, str))
    {
        // The first line represent`s how many attackers there are
        if (counter == 0)
        {
            numberOfAttackers = stoi(str);
            *attackersDegree = new int [numberOfAttackers];
        }
        else
        {
            // Each line represent`s one attacker, ordered, with it`s degree
            // So the length of node elements is 2
            vector<string> nodeElements;
            split(str, ' ', back_inserter(nodeElements));
            (*attackersDegree)[counter - 1] = stoi(nodeElements[1]);
        }

        counter += 1;
    }
}

int main (int argc, char * argv[])
{
    // Input: Path to graph G and way it`s structured, Path to information about the attackers, output folder
    if(argc != 5)
    {
        cout << "Wrong number of parameters." << endl;
        return EXIT_FAILURE;
    }

    string graphFilePath(argv[1]);
    string attackersInformationPath(argv[2]);
    string outputFolderPath(argv[3]);
    string graphStructure(argv[4]);

    int numericGraphStructure = getGraphStructure(graphStructure);

    // Read graph
    unordered_map<string, unordered_set<string>> graph = readGraph(numericGraphStructure, graphFilePath);

    // Read attackers information (we need the node`s degrees ordered by the attackers)

    int * attackersDegree;
    getAttackersDegree(attackersInformationPath, &attackersDegree);

    for (int i = 0; i < 22; i++)
    {
        std::cout << attackersDegree[i] << std::endl;
    }

    delete attackersDegree;

    std::cout << "Hello, World!" << std::endl;
    return 0;
}