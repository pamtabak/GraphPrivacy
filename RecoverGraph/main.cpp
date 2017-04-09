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

    return graph;
}

vector<int> getAttackersDegree(string filePath, int &numberOfAttackers)
{
    vector<int> attackersDegree;

    ifstream file(filePath);
    string str;
    int counter = 0;
    while (getline(file, str))
    {
        // The first line represent`s how many attackers there are
        if (counter == 0)
        {
            numberOfAttackers = stoi(str);
        }
        else
        {
            // Each line represent`s one attacker, ordered, with it`s degree
            // So the length of node elements is 2
            vector<string> nodeElements;
            split(str, ' ', back_inserter(nodeElements));
            attackersDegree.push_back(stoi(nodeElements[1]));
        }

        counter += 1;
    }

    return attackersDegree;
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
    // The attackers file has the amount of attackers and, on each line ordered, each attacker`s degree
    int numberOfAttackers;
    vector<int> attackersDegree = getAttackersDegree(attackersInformationPath, numberOfAttackers);

    vector<vector<string> > tree;

    // start tree with dummy node
    // then, each node that has the same degree as attackersDegree[0] will be connected to the root
    for (unsigned i = 0; i < graph.bucket_count(); ++i)
    {
        for (auto local_it = graph.begin(i); local_it!= graph.end(i); ++local_it)
        {
            if (local_it->second.size() == attackersDegree[0])
            {
                vector<string> path;
                path.push_back(local_it->first);
                tree.push_back(path);
            }
        }
    }

    int iterations = 1;
    while (iterations != numberOfAttackers)
    {
        vector<vector<string> > newPaths;
        for (int i = 0; i < tree.size(); i++)
        {
            vector<string> path = tree[i];
            // Getting all neighbors from graph node (last node from path)
            unordered_set<string> nodeNeighbors = graph[path[path.size() - 1]];
            for (auto neighborIt = nodeNeighbors.begin(); neighborIt != nodeNeighbors.end(); ++neighborIt)
            {
                // Checking if node has the correct degree and it`s not already on the path
                if ((graph[*neighborIt].size() == attackersDegree[iterations]) && (find(path.begin(), path.end(), *neighborIt) == path.end()))
                {
                    vector<string> tempPath = path;
                    tempPath.push_back(*neighborIt);
                    newPaths.push_back(tempPath);
                }
            }
        }
        tree = newPaths;
        iterations += 1;
    }

    cout << tree.size() << endl;

    for (int i = 0; i < tree.size(); i++)
    {
        cout << " " << endl;
        for (int j = 0; j < tree[i].size(); j++)
        {
            cout << tree[i][j]  << ",";
        }
    }

    return 0;
}