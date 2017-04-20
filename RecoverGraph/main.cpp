#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <vector>
#include "Node.hpp"
#include <algorithm>
#include <cctype>
#include <chrono>

using namespace std;

chrono::high_resolution_clock::time_point startTime;

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
                if (nodeElements.size() != 2)
                {
                    continue;
                }
                graph[nodeElements[0]].insert(nodeElements[1]);
                graph[nodeElements[1]].insert(nodeElements[0]);
            }
            break;
    }

    return graph;
}

// TO DO: total degree, external degree, (neighbor`s degrees)
vector<Node> getAttackersInfo(string filePath, int &numberOfAttackers)
{
    vector<Node> attackersDegree;

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
            Node * node = new Node();

            vector<string> attackerInfo;
            split(str, '(', back_inserter(attackerInfo));
            if (attackerInfo.size() != 2)
            {
                continue;
            }

            vector<string> nodeDegreeInfo;
            split(attackerInfo[0], ',', back_inserter(nodeDegreeInfo));
            if (nodeDegreeInfo.size() != 3)
            {
                continue;
            }
            node->setDegree(stoi(nodeDegreeInfo[1]));

            vector<string> neighborsDegreeInfo;
            vector<int> attackerNeighborDegree;
            string neighborInfo = attackerInfo[1];
            split(neighborInfo, ',', back_inserter(neighborsDegreeInfo));
            for (int i = 0 ; i < neighborsDegreeInfo.size(); i++)
            {
                if (neighborsDegreeInfo[i].empty() or ")" == neighborsDegreeInfo[i])
                {
                    continue;
                }
                attackerNeighborDegree.push_back(stoi(neighborsDegreeInfo[i]));
            }
            sort(attackerNeighborDegree.begin(), attackerNeighborDegree.end());

            node->setAttackerNeighborDegree(attackerNeighborDegree);

            vector<string> nodeElements;
            split(str, ' ', back_inserter(nodeElements));

            attackersDegree.push_back(*node);
            delete node;
        }

        counter += 1;
    }

    return attackersDegree;
}

bool includes (vector<int> first, vector<int> second)
{
    // both vectors are sorted
    // second.size() <= first.size()
    int firstIndexMatch = -1;
    for (int i = 0; i < first.size(); i++)
    {
        if (first[i] != second[0])
        {
            continue;
        }
        firstIndexMatch = i;
        break;
    }

    if (firstIndexMatch == -1 or (firstIndexMatch + second.size()) > first.size())
    {
        return false;
    }

    int secondIndex = 0;
    while (secondIndex < second.size())
    {
        int c = count (second.begin(), second.end(), second[secondIndex]);
        if (c > count (first.begin(), first.end(), second[secondIndex]))
        {
            return false;
        }
        secondIndex += c;
    }
    return true;
}

vector<string> findOrderedAttackers (unordered_map<string, unordered_set<string>> graph, vector<Node> attackersDegree, int numberOfAttackers)
{
    vector<vector<string> > tree;

    // start tree with dummy node
    // then, each node that has the same degree as attackersDegree[0] will be connected to the root
    for (unsigned i = 0; i < graph.bucket_count(); ++i)
    {
        for (auto local_it = graph.begin(i); local_it!= graph.end(i); ++local_it)
        {
            // First, we check if the degree is the one we are searching for
            if (local_it->second.size() == attackersDegree[0].getDegree())
            {
                // Then we check if this node neighbor`s degrees contains the attacker`s degrees that the
                // attacker_0 has
                vector<int> nodeNeighborDegree;
                for (unsigned i = 0; i < local_it->second.bucket_count(); ++i)
                {
                    for (auto it = local_it->second.begin(i); it != local_it->second.end(i); ++it)
                    {
                        nodeNeighborDegree.push_back(graph[*it].size());
                    }
                }

                sort(nodeNeighborDegree.begin(), nodeNeighborDegree.end());
                if (includes(nodeNeighborDegree, attackersDegree[0].getAttackerNeighborDegree()))
                {
                    vector<string> path;
                    path.push_back(local_it->first);
                    tree.push_back(path);
                }
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
                if ((graph[*neighborIt].size() == attackersDegree[iterations].getDegree()) && (find(path.begin(), path.end(), *neighborIt) == path.end()))
                {
                    // Getting neighbor`s neighbors
                    vector<int> nodeNeighborDegree;
                    for (unsigned i = 0; i < graph[*neighborIt].bucket_count(); ++i)
                    {
                        for (auto it = graph[*neighborIt].begin(i); it != graph[*neighborIt].end(i); ++it)
                        {
                            nodeNeighborDegree.push_back(graph[*it].size());
                        }
                    }

                    sort(nodeNeighborDegree.begin(), nodeNeighborDegree.end());
                    // Attackers degree is ordered by the attacker`s labels.
                    if (includes(nodeNeighborDegree, attackersDegree[iterations].getAttackerNeighborDegree()))
                    {
                        vector<string> tempPath = path;
                        tempPath.push_back(*neighborIt);
                        newPaths.push_back(tempPath);
                    }
                }
            }
            // clearing memory
            nodeNeighbors = unordered_set<string>();
        }
        tree = newPaths;
        iterations += 1;
    }

    if (tree.size() != 1)
    {
        return vector<string>();
    }

    return tree[0];
}

void getNodesIdentity (unordered_map<string, unordered_set<string>> subgraph, unordered_map<string, unordered_set<string>> graph, unordered_map<string, string> &identity, unordered_map<string, string> &anonimized)
{
    for (unsigned i = 0; i < subgraph.bucket_count(); ++i)
    {
        for (auto local_it = subgraph.begin(i); local_it!= subgraph.end(i); ++local_it)
        {
            // if node has not been identified yet
            if (identity.find(local_it->first) == identity.end())
            {
                unordered_set<string> attackers;

                for (auto neighborIt = local_it->second.begin(); neighborIt != local_it->second.end(); ++neighborIt)
                {
                    if (identity.find(*neighborIt) != identity.end())
                    {
                        string neighborLabel = *neighborIt;
                        if (neighborLabel.find("attacker_") != string::npos)
                        {
                            attackers.insert(neighborLabel);
                        }
                    }
                }

                if (attackers.size() > 0)
                {
                    // For all subgraph nodes yet to be identified, check if
                    // subgraph_degree <= graph_degree. If not, don`t even bother checking
                    for (unsigned j = 0; j < graph.bucket_count(); ++j)
                    {
                        for (auto it = graph.begin(j); it != graph.end(j); ++it)
                        {
                            if (anonimized.find(it->first) == identity.end())
                            {
                                if (local_it->second.size() <= it->second.size())
                                {
                                    bool hasSameAttackers  = true;
                                    int  numberOfAttackers = 0;

                                    // getting all attackers from this node`s neighbors
                                    for (auto neighborIt = it->second.begin(); neighborIt != it->second.end(); ++neighborIt)
                                    {
                                        if (anonimized.find(*neighborIt) != identity.end())
                                        {
                                            string neighborLabel = anonimized[*neighborIt];
                                            if (neighborLabel.find("attacker_") != string::npos)
                                            {
                                                numberOfAttackers += 1;
                                                if (attackers.find(neighborLabel) == attackers.end())
                                                {
                                                    hasSameAttackers = false;
                                                    break;
                                                }
                                            }
                                        }
                                    }

                                    if (hasSameAttackers && numberOfAttackers == attackers.size())
                                    {
                                        // This only works if no-target nodes are not associated with attackers
                                        identity[local_it->first] = it->first;
                                        anonimized[it->first]     = local_it->first;
                                    }
                                }
                            }
                        }
                    }
                }
                attackers = unordered_set<string>();
            }
        }
    }
}

int main (int argc, char * argv[])
{
    // Input: Path to graph G and way it`s structured, Path to information about the attackers, output folder
    if(argc != 6)
    {
        cout << "Wrong number of parameters." << endl;
        return EXIT_FAILURE;
    }

    // Execution has started
    startTime = chrono::high_resolution_clock::now();

    string graphFilePath(argv[1]);
    string attackersInformationPath(argv[2]);
    string outputFolderPath(argv[3]);
    string graphStructure(argv[4]);
    string subgraphFilePath(argv[5]);

    int numericGraphStructure = getGraphStructure(graphStructure);

    // Read graph
    unordered_map<string, unordered_set<string>> graph    = readGraph(numericGraphStructure, graphFilePath);
    unordered_map<string, unordered_set<string>> subgraph = readGraph(numericGraphStructure, subgraphFilePath);

    // Read attackers information (we need the node`s degrees ordered by the attackers)
    // The attackers file has the amount of attackers and, on each line ordered, each attacker`s degree
    int numberOfAttackers;
    vector<Node> attackersDegree = getAttackersInfo(attackersInformationPath, numberOfAttackers);

    vector<string> path = findOrderedAttackers (graph, attackersDegree, numberOfAttackers);
    if (path.size() == 0)
    {
        cout << "failed" << endl;
        return -1;
    }

    cout << " " << endl;

    // real label mapped to anonimized label
    unordered_map<string, string> identity;

    // anonimized label mapped to real label
    unordered_map<string, string> anonimized;
    for (int j = 0; j < path.size(); j++)
    {
        // attackers displayed in order
        anonimized[path[j]] = "attacker_" + to_string(j);
        identity["attacker_" + to_string(j)] = path[j];
        cout << "attacker_" + to_string(j)  << " ";
    }
    cout << "" << endl;

    getNodesIdentity (subgraph, graph, identity, anonimized);

    // clearing memory
    anonimized = unordered_map<string, string>();
    identity         = unordered_map<string, string>();
    graph            = unordered_map<string, unordered_set<string>>();

    // End of execution
    chrono::high_resolution_clock::time_point endTime = chrono::high_resolution_clock::now();
    chrono::duration<double> endTimeSpan              = chrono::duration_cast<chrono::duration<double> >(endTime - startTime);
    cout << "end: " << endTimeSpan.count() << " secs" << endl;

    return 0;
}