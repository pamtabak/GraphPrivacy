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

using namespace std;

chrono::high_resolution_clock::time_point startTime;

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

HashTable<string, Node> initializeAttackers (int d0, int d1, int kNewAccounts)
{
    HashTable<string, Node> newAccounts;

    // Initializing new accounts
    for (int i = 0; i < kNewAccounts; i++)
    {
        Node * node = new Node();
        newAccounts.set("attacker_" + to_string(i), *node);
        delete node;
    }

    default_random_engine generator;
    generator.seed(kNewAccounts);
    uniform_int_distribution<int> distribution(d0, d1); // is this generating an independent distribution???

    for (int i = 0; i < kNewAccounts; i++)
    {
        string nodeLabel = "attacker_" + to_string(i);
        Node node = newAccounts.get(nodeLabel);

        // Choosing an external degree (connections with G - H) independently and uniformly at random from [d0, d1]
        int maximumExternalDegree = distribution(generator);
        node.setMaximumExternalDegree(maximumExternalDegree);

        cout << nodeLabel << ", " << maximumExternalDegree << endl;

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

            // is this independently??
            if (rand() % 2 == 1)
            {
                string neighborLabel = "attacker_" + to_string(j);
                node.addNeighbor(neighborLabel);
                node.setDegree(node.getDegree() + 1);

                Node neighborNode = newAccounts.get(neighborLabel);
                neighborNode.setDegree(neighborNode.getDegree() + 1);
                newAccounts.set(neighborLabel, neighborNode);
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

vector<list<string> > generateAllPossibleSubsets (int maximumSubsetSize, vector<string> newAccountKeys)
{
    // Generating all possible subsets from new account keys that have size at most equals to maximum subset size
    vector<list<string> > allPossibleSubsets;
    list<string> lt;
    for (int i = 1; i <= maximumSubsetSize; i++)
    {
        subset(newAccountKeys, newAccountKeys.size(), i, 0, lt, allPossibleSubsets);
    }

    return allPossibleSubsets;
}

void generateTargetedNodesSet (vector<string> targetedNodes, HashTable<string, Node> &newAccounts, int maximumSubsetSize)
{
    // Generates all subsets with size <= maximum subset size
    vector<list<string> > allPossibleSubsets = generateAllPossibleSubsets(maximumSubsetSize, newAccounts.getKeys());

    for (int i = 0; i < targetedNodes.size(); i++)
    {
        bool generatedSubset = false;
        while (!generatedSubset)
        {
            // Getting random subset from all possible subsets
            list<string> subset = allPossibleSubsets[rand() % allPossibleSubsets.size()];

            // Checking if this subset can be used
            // All of its nodes need to be able to create external edges, so it can`t have already reached it`s maximum
            bool subsetCanBeUsed = true;
            for (list<string>::iterator it = subset.begin(); it != subset.end(); ++it)
            {
                Node node = newAccounts.get(*it);
                if (node.getExternalDegree() == node.getMaximumExternalDegree())
                {
                    subsetCanBeUsed = false;
                    break;
                }
            }

            if (subsetCanBeUsed)
            {
                generatedSubset = true;
                // Updating external degree until it reaches maximum external degree
                for (list<string>::iterator it = subset.begin(); it != subset.end(); ++it)
                {
                    Node node = newAccounts.get(*it);
                    node.setExternalDegree(node.getExternalDegree() + 1);
                    node.setDegree(node.getDegree() + 1);
                    node.addNeighbor(targetedNodes[i]);
                    newAccounts.set(*it, node);
                }
            }

            // Removing subset from all possible subsets
            allPossibleSubsets.erase(std::remove(allPossibleSubsets.begin(), allPossibleSubsets.end(), subset), allPossibleSubsets.end());
        }
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

void generateTargetedNodesSet (vector<string> targetedNodes, HashTable<string, Node> &newAccounts)
{
    // We save all different subsets here
    unordered_set<vector<string>, VectorHash> differentSubsets;
    HashTable<string, vector<string>> targetedNodesSubsets;
    vector<string> attackers = newAccounts.getKeys();

    // Defines whether a targeted node already has its attacker`s subset to connect
    bool markerVector[targetedNodes.size()];
    // Initializing marker vector with all positions as false
    fill_n(markerVector, targetedNodes.size(), false);

    while (!isAllTrue(markerVector, targetedNodes.size()))
    {
        // This is the expected to happen, but it`s not mandatory
        if (targetedNodes.size() >= newAccounts.size())
        {
            for (int i = 0; i < targetedNodes.size(); i++)
            {
                if (markerVector[i])
                {
                    // This node already has it`s subset
                    continue;
                }

                vector<string> newSubset = targetedNodesSubsets.get(targetedNodes[i]);
                if (newSubset.size() == 0)
                {
                    newSubset.push_back(attackers[i % newAccounts.size()]);
                }
                else
                {
                    // adding the last one added + 1
                    string lastAttackerAdded = newSubset[newSubset.size() - 1];
                    ptrdiff_t pos = find(attackers.begin(), attackers.end(), lastAttackerAdded) - attackers.begin();
                    if (pos == attackers.size() - 1) { pos = 0; }
                    newSubset.push_back(attackers[pos + 1]);
                }

                targetedNodesSubsets.set(targetedNodes[i], newSubset);

                unordered_set<vector<string>, VectorHash>::const_iterator got = differentSubsets.find (newSubset);
                if ( got == differentSubsets.end() )
                {
                    // no other node has the same subset
                    markerVector[i] = true;
                }
            }
        }
        else
        {
            // TO DO??
        }
    }

    // Generate d0 and d1
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
    string outputFilePath(argv[3]);
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

    // Choose constants 1 <= d0 <= d1 = O (log2 (n))
    double logNumberOfNodes = log2(numberOfNodes);
    int    d1               = (int) logNumberOfNodes;
    if (d1 == 0) {d1 = 1;} // In case the number of nodes is too low

    int d0 = (rand() % d1) + 1; // random number between 1 and d1
    cout << "[d0, d1]: [" << d0 << ", " << d1 << "]" << endl;

    //  Create k = (2 + y) log2(n) -> new accounts, small constant y > 0
    int          y            = 3; // should this constant be a param?
    unsigned int kNewAccounts = (unsigned int) ((2 + y) * logNumberOfNodes);
    cout << "number of attackers: " << kNewAccounts << endl;

    // Initializing attackers: Setting edges between the attackers and maximum external degree
    HashTable<string, Node> newAccounts = initializeAttackers(d0, d1, kNewAccounts);

    // For each targeted node Wj, we choose a set Nj (contained in newAccounts), such that all Nj are distinct,
    // each Nj has size at most c, and each Xi (attackers/new account) appears at most Di (it`s external degree)
    // of the sets Nj
    int maximumSubsetSize = 3; // c
    generateTargetedNodesSet (targetedNodes, newAccounts, maximumSubsetSize);

    // Adding arbitrary edges from H to G - H, so that each attacker node has exactly Di edges to G - H
    // And writing it to file - new accounts and it`s edges - sub graph H and edges connecting it to G
    addExtraEdgesAndWriteOutputFiles (outputFilePath, kNewAccounts, newAccounts, nodes, targetedNodes);

    // End of execution
    chrono::high_resolution_clock::time_point endTime = chrono::high_resolution_clock::now();
    chrono::duration<double> endTimeSpan              = chrono::duration_cast<chrono::duration<double> >(endTime - startTime);
    printf("end: %lf secs\n", endTimeSpan.count());

    return 0;
}