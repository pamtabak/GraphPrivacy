#include <iostream>
#include "HashTable.hpp"
#include "Node.hpp"
#include <random>
#include <unordered_set>
#include <string>
#include <sstream>
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <list>
#include <algorithm>
#include <chrono>
#include <vector>

using namespace std;

chrono::high_resolution_clock::time_point startTime;

double epsilon = 0.0001;

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

// Reads input graph and returns it`s node`s labels and number of nodes
unordered_set<string> readGraph (int numericGraphStructure, string filePath)
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

// Reads file and return all targeted nodes labels
vector<string> readTargetedNodes (string filePath)
{
    vector<string> targetedNodes;

    ifstream file(filePath);
    string str;
    while (getline(file, str))
    {
        targetedNodes.push_back(str);
    }

    return targetedNodes;
}

HashTable<string, Node> initializeAttackers (int kNewAccounts)
{
    HashTable<string, Node> newAccounts;

    // Initializing new accounts
    for (int i = 0; i < kNewAccounts; i++)
    {
        Node * node = new Node();
        newAccounts.set("attacker_" + to_string(i), *node);
        delete node;
    }

    for (int i = 0; i < kNewAccounts; i++)
    {
        string nodeLabel = "attacker_" + to_string(i);
        Node node = newAccounts.get(nodeLabel);

        // Including each (Xi, Xi+1)
        // Generating the remain edges inside H
        // Since it`s an undirected graph, we only need to add the edge once
        string neighborLabel;
        if (i != (kNewAccounts - 1))
        {
            neighborLabel = "attacker_" + to_string(i + 1);
        }
        else
        {
            // Adding the edge to close the circle connecting the attackers
            // The last attacker is connected with the first attacker
            neighborLabel = "attacker_" + to_string(0);
        }
        node.addNeighbor(neighborLabel);
        node.setDegree(node.getDegree() + 1);

        // But we need to update both node`s degrees
        Node neighborNode = newAccounts.get(neighborLabel);
        neighborNode.setDegree(neighborNode.getDegree() + 1);
        newAccounts.set(neighborLabel, neighborNode);

        // Include each other (Xi, Xj) independently with probability 1/2
        for (int j = (i + 2); j < kNewAccounts; j++)
        {
            if (i == 0 && j == kNewAccounts - 1)
            {
                // This edge already exists
                continue;
            }

            if (rand() % 2 == 1)
            {
                string neighborLabel2 = "attacker_" + to_string(j);
                node.addNeighbor(neighborLabel2);
                node.setDegree(node.getDegree() + 1);

                Node neighborNode2 = newAccounts.get(neighborLabel2);
                neighborNode2.setDegree(neighborNode2.getDegree() + 1);
                newAccounts.set(neighborLabel2, neighborNode2);
            }
        }
        newAccounts.set(nodeLabel, node);
    }

    return newAccounts;
}

// generates all subsets (with size specified) from an array
void subset(vector<string> arr, int size, int left, int index, list<string> &l, vector<list<string> > & subsets)
{
    if(left==0)
    {
        // new subset generated
        subsets.push_back(l);
        return;
    }

    for(int i=index; i<size;i++)
    {
        l.push_back(arr[i]);
        subset(arr,size,left-1,i+1,l, subsets);
        l.pop_back();
    }
}

void addExtraEdgesAndWriteOutputFiles (string outputFilePath, int kNewAccounts, HashTable<string, Node> &newAccounts, unordered_set<string> nodes, vector<string> targetedNodes)
{
    ofstream file, degreeFile;
    file.open(outputFilePath + "subgraph.txt", std::fstream::out | std::fstream::trunc);
    degreeFile.open(outputFilePath + "degree.txt",   std::fstream::out | std::fstream::trunc);

    // Writing number of attackers to degree file
    degreeFile << kNewAccounts << endl;

    /* Generating the remaining extra edges (until every attacker achieves it`s stipulated maximum external degree)
     * between attackers and non targeted nodes */
    for (int i = 0; i < targetedNodes.size(); i++)
    {
        nodes.erase(targetedNodes[i]);
    }

    for (int k = 0; k < kNewAccounts; k++)
    {
        string nodeLabel = "attacker_" + to_string(k);
        Node node = newAccounts.get(nodeLabel);
        while (node.getMaximumExternalDegree() != node.getExternalDegree())
        {
            auto random_it = next(std::begin(nodes), rand() % nodes.size());
            node.addNeighbor(*random_it); // any node inside nodes unordered_set
            node.setExternalDegree(node.getExternalDegree() + 1);
            node.setDegree(node.getDegree() + 1);
        }
        newAccounts.set(nodeLabel, node);

        // Writing it to file
        unordered_set<string> neighbors = node.neighbors;
        for (unsigned i = 0; i < neighbors.bucket_count(); ++i)
        {
            for (auto local_it = neighbors.begin(i); local_it != neighbors.end(i); ++local_it)
            {
                file << nodeLabel << " " << *local_it << "\n";
            }
        }
        degreeFile << nodeLabel << ", " << node.getDegree() << "\n";
    }
}

struct VectorHash {
    size_t operator()(const std::vector<string>& v) const {
        std::hash<string> hasher;
        size_t seed = 0;
        for (string i : v) {
            seed ^= hasher(i) + 0x9e3779b9 + (seed<<6) + (seed>>2);
        }
        return seed;
    }
};

// Checks if all array positions are set with true. If so, return true. Else, return false
bool isAllTrue (bool markerVector[], int markerVectorSize)
{
    for (int i = 0; i < markerVectorSize; i++)
    {
        if (markerVector[i] == false)
        {
            return  false;
        }
    }

    return true;
}

int findD1 (int d0, int maxD)
{
    // d1 > d0

    // pow(d1,2) - d1*maxD - d1*d0 + d0*maxD = d0*(1 - epsilon)
    // pow (d1,2) - d1(maxD + d0) + d0(maxD - 1 + epsilon) = 0
    // pow (d1,2) + d1(-d0 - maxD) + d0(maxD - 1 + epsilon) = 0

    double rootA, rootB = 0.0;
    try {
        rootA = (-(-d0 - maxD) - sqrt(pow((-d0 - maxD), 2) - 4 * 1 * (maxD - 1 + epsilon))) / 2;
        rootB = (-(-d0 - maxD) + sqrt(pow((-d0 - maxD), 2) - 4 * 1 * (maxD - 1 + epsilon))) / 2;
    }
    catch (int e)
    {
        // some imaginary number was found (?)
    }

    if (rootA > 0.0 && rootA > d0)
    {
        // rounding number. Ex: rootA is 54.6, so we return 55. If rootA is 54.4, we return 54
        return (int) (rootA + 0.5);
    }
    if (rootB > 0.0 && rootB > d0)
    {
        return (int) (rootB + 0.5);
    }

    return 0;
}

void generateTargetedNodesSet (vector<string> targetedNodes, HashTable<string, Node> &newAccounts)
{
    bool couldGenerateSets = false;
    while (!couldGenerateSets)
    {
        // We save all different subsets here
        unordered_set<vector<string>, VectorHash> differentSubsets;
        HashTable<string, vector<string>> targetedNodesSubsets;
        vector<string> attackers = newAccounts.getKeys();

        // Defines whether a targeted node already has its attacker`s subset to connect
        bool * markerVector = new bool[targetedNodes.size()];
        // Initializing marker vector with all positions as false
        for (int i = 0; i < targetedNodes.size(); i++)
        {
            markerVector[i] = false;
        }

        while (!isAllTrue(markerVector, targetedNodes.size()))
        {
            for (int i = 0; i < targetedNodes.size(); i++)
            {
                if (markerVector[i])
                {
                    // This node already has it`s subset
                    continue;
                }

                string attackerLabel;
                vector<string> newSubset = targetedNodesSubsets.get(targetedNodes[i]);
                if (newSubset.size() == 0)
                {
                    attackerLabel = attackers[i % newAccounts.size()];
                    newSubset.push_back(attackerLabel);
                }
                else
                {
                    // Getting the last attacker added and adding the next attacker to it
                    // If Xi had been added, we now add Xi+1
                    string lastAttackerAdded = newSubset[newSubset.size() - 1];
                    int num;
                    sscanf(lastAttackerAdded.c_str(), "%*[^_]_%d", &num);
                    if (num == attackers.size() - 1)
                        num = 0;
                    else
                        num += 1;

                    attackerLabel = "attacker_" + to_string(num);
                    newSubset.push_back(attackerLabel);
                }

                // Updating attacker's external degree
                Node node = newAccounts.get(attackerLabel);
                node.setExternalDegree(node.getExternalDegree() + 1);
                node.setDegree(node.getDegree() + 1);
                node.addNeighbor(targetedNodes[i]);
                newAccounts.set(attackerLabel, node);

                targetedNodesSubsets.set(targetedNodes[i], newSubset);

                unordered_set<vector<string>, VectorHash>::const_iterator got = differentSubsets.find (newSubset);
                if ( got == differentSubsets.end() )
                {
                    // no other node has the same subset
                    differentSubsets.insert(newSubset);
                    markerVector[i] = true;
                }
            }
        }

        delete[] markerVector;

        // Once all targeted nodes have a subset (of attackers), we generate a maximum external degree
        // for each attacker
        vector<pair<string, int>> attackersExternalDegree;
        for (int i = 0; i < attackers.size(); i++)
        {
            Node node = newAccounts.get(attackers[i]);
            pair<string, int> pair1 (attackers[i], node.getExternalDegree());
            attackersExternalDegree.push_back(pair1);
        }

        // Sorting vector by the external degree
        sort(attackersExternalDegree.begin(), attackersExternalDegree.end(),[](const pair<string, int>& p1, const pair<string, int>& p2) {
            return p1.second < p2.second; });

        // Generating maximum external degree distribution
        // 1 <= d0 <= log2(numberOfNodes) == newAccounts.size()
        int d0   = (rand() % newAccounts.size()) + 1; // random number between 1 and log2(numberOfNodes)
        int maxD = attackersExternalDegree[attackersExternalDegree.size() - 1].second; // biggest empirical external degree
        int d1 = findD1(d0, maxD);
        //d1 = d1 * ((newAccounts.size() - 1) / d0); // d1 is O(log2(numberOfNodes), smaller than pow(log2(),2);

        default_random_engine generator;
        generator.seed(newAccounts.size());
        uniform_int_distribution<int> distribution(d0, d1);

        // Generate a list with n numbers (n = newAccounts.size()) between d0 and d1
        vector<int> maximumExternalDegreeEmpiricalDistribution;
        for (int i = 0; i < newAccounts.size(); i++)
        {
            maximumExternalDegreeEmpiricalDistribution.push_back(distribution(generator));
        }
        sort(maximumExternalDegreeEmpiricalDistribution.begin(), maximumExternalDegreeEmpiricalDistribution.end());

        cout << "maximum external degree: " << maximumExternalDegreeEmpiricalDistribution[newAccounts.size() - 1] << endl;

        // Try to set a maximum external degree to each attacker, considering the external degree they already have
        for (int i = 0; i < newAccounts.size(); i++)
        {
            if (attackersExternalDegree[i].second > maximumExternalDegreeEmpiricalDistribution[i])
            {
                break;
            }
            Node node = newAccounts.get(attackersExternalDegree[i].first);
            node.setMaximumExternalDegree(maximumExternalDegreeEmpiricalDistribution[i]);
            newAccounts.set(attackersExternalDegree[i].first, node);
        }
        couldGenerateSets = true;
    }
}

int main (int argc, char * argv[])
{
    // Execution has started
    startTime = chrono::high_resolution_clock::now();

    // Output: attackers and it`s edges. Attackers degrees
    // Input: Path to graph G and way it`s structured, Path to list of target nodes
    if(argc != 5)
    {
        cout << "Wrong number of parameters." << endl;
        return EXIT_FAILURE;
    }

    string graphFilePath(argv[1]);
    string targetedNodesFilePath(argv[2]);
    string outputFolderPath(argv[3]);
    string graphStructure(argv[4]);
    int numericGraphStructure = getGraphStructure(graphStructure);

    // Reading graph (we only need node`s labels and number of nodes)
    unordered_set<string> nodes         = readGraph (numericGraphStructure, graphFilePath);
    int                   numberOfNodes = (int) nodes.size();
    cout << "number of nodes: " << numberOfNodes << endl;

    // Reading targeted nodes
    vector<string> targetedNodes = readTargetedNodes(targetedNodesFilePath);
    cout << "number of targeted nodes: " << targetedNodes.size() << endl;

    /* initialize random seed: */
    srand (time(NULL));

    //  Create k = (2 + y) log2(n) -> new accounts, small constant y > 0
    int          y            = 3; // should this constant be a param?
    unsigned int kNewAccounts = (unsigned int) ((2 + y) * log2(numberOfNodes));
    cout << "number of attackers: " << kNewAccounts << endl;

    // Initializing attackers: Setting edges between the attackers and maximum external degree
    HashTable<string, Node> newAccounts = initializeAttackers(kNewAccounts);

    // For each targeted node Wj, we choose a set Nj (contained in newAccounts), such that all Nj are distinct,
    // Once the sets are defined, we give each attacker a maximum external degree
	generateTargetedNodesSet(targetedNodes, newAccounts);

    // Adding arbitrary edges from H to G - H, so that each attacker node has exactly Di edges to G - H
    // And writing it to file - new accounts and it`s edges - sub graph H and edges connecting it to G
	addExtraEdgesAndWriteOutputFiles(outputFolderPath, kNewAccounts, newAccounts, nodes, targetedNodes);

    // End of execution
    chrono::high_resolution_clock::time_point endTime = chrono::high_resolution_clock::now();
    chrono::duration<double> endTimeSpan              = chrono::duration_cast<chrono::duration<double> >(endTime - startTime);
    printf("end: %lf secs\n", endTimeSpan.count());

    return 0;
}