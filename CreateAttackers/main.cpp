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
#include <math.h>

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

        Node neighborNode = newAccounts.get(neighborLabel);
        neighborNode.setDegree(neighborNode.getDegree() + 1);
        neighborNode.addNeighbor(nodeLabel);
        newAccounts.set(neighborLabel, neighborNode);

        // Include each other (Xi, Xj) independently with probability 1/2
        for (int j = (i + 2); j < kNewAccounts; j++)
        {
            if (i == 0 && j == kNewAccounts - 1)
            {
                // This edge already exists
                continue;
            }

            // G(n,p), p = 1/2
            if (rand() % 2 == 1 )
            {
                string neighborLabel2 = "attacker_" + to_string(j);
                node.addNeighbor(neighborLabel2);
                node.setDegree(node.getDegree() + 1);

                Node neighborNode2 = newAccounts.get(neighborLabel2);
                neighborNode2.addNeighbor(nodeLabel);
                neighborNode2.setDegree(neighborNode2.getDegree() + 1);
                newAccounts.set(neighborLabel2, neighborNode2);
            }
        }
        newAccounts.set(nodeLabel, node);
    }

    return newAccounts;
}

void addExtraEdges (HashTable<string, Node> &newAccounts, unordered_set<string> nodes, vector<string> targetedNodes)
{
    /* Generating the remaining extra edges (until every attacker achieves it`s stipulated maximum external degree)
     * between attackers and non targeted nodes */
    for (int i = 0; i < targetedNodes.size(); i++)
    {
        nodes.erase(targetedNodes[i]);
    }

    // nodes are the all nodes that are not targets from the original graph

    // each non target nodes can be only attached to one attacker
    for (int k = 0; k < newAccounts.size(); k++)
    {
        string nodeLabel = "attacker_" + to_string(k);
        Node node = newAccounts.get(nodeLabel);
        if (nodes.size() == 0)
        {
            break;
        }

        while (node.getMaximumExternalDegree() != node.getExternalDegree() && nodes.size() > 0)
        {
            auto random_it = next(std::begin(nodes), rand() % nodes.size());
            node.addNeighbor(*random_it); // any node inside nodes unordered_set
            node.setExternalDegree(node.getExternalDegree() + 1);
            node.setDegree(node.getDegree() + 1);
            nodes.erase(*random_it);
        }

        newAccounts.set(nodeLabel, node);
    }

    // clearing memory
    nodes = std::unordered_set<string>();
}

void writeFile (string outputFilePath, HashTable<string, Node> newAccounts)
{
    ofstream file, degreeFile;
    file.open(outputFilePath + "subgraph.txt", std::fstream::out | std::fstream::trunc);
    degreeFile.open(outputFilePath + "degree.txt",   std::fstream::out | std::fstream::trunc);

    // Writing number of attackers to degree file
    degreeFile << newAccounts.size() << endl;

    for (int k = 0; k < newAccounts.size(); k++)
    {
        string nodeLabel = "attacker_" + to_string(k);
        Node node = newAccounts.get(nodeLabel);

        // Writing it to file
        list<string> neighbors = node.neighbors;
        string neighborsDegree = "";
        for (list<string>::iterator it=neighbors.begin(); it != neighbors.end(); ++it)
        {
                file << nodeLabel << " " << *it << "\n";

                unordered_map<string, Node>::const_iterator got = newAccounts.hashMap.find(*it);
                if (got != newAccounts.hashMap.end())
                {
                    Node neighborNode = got->second;
                    neighborsDegree += to_string(neighborNode.getDegree()) + ",";
                }
        }
        degreeFile << nodeLabel << "," << node.getDegree() << "," << node.getExternalDegree() << ",(" << neighborsDegree << ")\n";
    }
}

//int findD1 (int d0, int maxD)
//{
//    // d1 > d0
//
//    // pow(d1,2) - d1*maxD - d1*d0 + d0*maxD = d0*(1 - epsilon)
//    // pow (d1,2) - d1(maxD + d0) + d0(maxD - 1 + epsilon) = 0
//    // pow (d1,2) + d1(-d0 - maxD) + d0(maxD - 1 + epsilon) = 0
//
//    double rootA, rootB = 0.0;
//    try {
//        rootA = (-(-d0 - maxD) - sqrt(pow((-d0 - maxD), 2) - 4 * 1 * (maxD - 1 + epsilon))) / 2;
//        rootB = (-(-d0 - maxD) + sqrt(pow((-d0 - maxD), 2) - 4 * 1 * (maxD - 1 + epsilon))) / 2;
//    }
//    catch (int e)
//    {
//        // some imaginary number was found (?)
//    }
//
//    if (rootA > 0.0 && rootA > d0)
//    {
//        // rounding number. Ex: rootA is 54.6, so we return 55. If rootA is 54.4, we return 54
//        return (int) (rootA + 0.5);
//    }
//    if (rootB > 0.0 && rootB > d0)
//    {
//        return (int) (rootB + 0.5);
//    }
//
//    return 0;
//}

void calculateMaximumExternalDegree (HashTable<string, Node> &newAccounts)
{
    // Once all targeted nodes have a subset (of attackers), we generate a maximum external degree
    // for each attacker
    default_random_engine generator;
    generator.seed(newAccounts.size());

    for (int i = 0; i < newAccounts.size(); i++)
    {
        string attackerLabel  = "attacker_" + to_string(i);
        Node   node           = newAccounts.get(attackerLabel);
        int    externalDegree = node.getExternalDegree();
        int    d1;

        if (externalDegree == 0)
        {
            externalDegree += 1;
            d1 = newAccounts.size(); // O(log n)
        }
        else
        {
            d1 = (((rand() % newAccounts.size()) + 1) * externalDegree);
        }

        uniform_int_distribution<int> distribution(externalDegree, d1);

        node.setMaximumExternalDegree(distribution(generator));
        newAccounts.set(attackerLabel, node);
    }
}

bool onlyOneAttacker (int * binarySubsets, int size)
{
    int attacker = 0;
    for (int i = 0; i < size; i++)
    {
        if (binarySubsets[i] == 1)
        {
            attacker += 1;
        }
    }

    if (attacker == 1)
    {
        return true;
    }
    return false;
}

void binarySum (int * add_one_binary, int numberOfAttackers, int* & binary_subsets)
{
    // adding one to the binary sequence
    int carry = 0;
    int * sum = new int[numberOfAttackers];;
    for(int j = numberOfAttackers - 1; j >= 0 ; j--)
    {
        sum[j] = ((binary_subsets[j] ^ add_one_binary[j]) ^ carry); // c is carry
        carry  = ((binary_subsets[j] & add_one_binary[j]) | (binary_subsets[j] & carry)) | (add_one_binary[j] & carry);
    }
    for (int j = 0; j < numberOfAttackers; j++)
    {
        binary_subsets[j] = sum[j];
    }

    delete[] sum;
}

void generateTargetedNodesSet (vector<string> targetedNodes, HashTable<string, Node> &newAccounts, int numberOfNodes)
{
    // we create an int array with size = number of attackers
    // We start with a "0000001" binary number. For each position, if it`s 1, then that attacker should be at the
    // subset for that target. At each iteration, we add one to the binary number
    // This way, all targets will have different attacker subsets
    int numberOfAttackers = newAccounts.size();
    int * binary_subsets = new int[numberOfAttackers];
    int * add_one_binary = new int[numberOfAttackers];

    // Making access to nodes faster
    Node * attackersArray = new Node[numberOfAttackers];
    for (int i = 0; i < numberOfAttackers; i++)
    {
        binary_subsets[i] = 0;
        add_one_binary[i] = 0;
        if (i == (numberOfAttackers - 1))
        {
            add_one_binary[i] = 1;
        }

        attackersArray[i] = newAccounts.get("attacker_" + to_string(i));
    }

    for (int i = 0; i < targetedNodes.size(); i++)
    {
        // targets must have, at least, 2 attackers (so we can attach non targets to attackers)
        if (onlyOneAttacker (binary_subsets, numberOfAttackers))
        {
            binarySum (add_one_binary, numberOfAttackers, binary_subsets);
            continue;
        }

        for (int j = 0; j < newAccounts.size(); j++)
        {
            if (binary_subsets[j] == 1)
            {
                // Updating attacker's external degree
                Node node = attackersArray[j];
                node.setExternalDegree(node.getExternalDegree() + 1);
                node.setDegree(node.getDegree() + 1);
                node.addNeighbor(targetedNodes[i]);
                attackersArray[j] = node;
            }
        }

        binarySum (add_one_binary, numberOfAttackers, binary_subsets);
    }

    for (int i = 0; i < newAccounts.size(); i++)
    {
        newAccounts.set("attacker_" + to_string(i), attackersArray[i]);
    }

    calculateMaximumExternalDegree(newAccounts);

    delete[] binary_subsets;
    delete[] add_one_binary;
    delete[] attackersArray;
}

int main (int argc, char * argv[])
{
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

    // Reading targeted nodes
    vector<string> targetedNodes = readTargetedNodes(targetedNodesFilePath);

    /* initialize random seed: */
    srand (time(NULL));

//    int kNewAccounts = (int) ceil(log2(numberOfNodes)) + 1;
//    if ((1.0*targetedNodes.size())/numberOfNodes > 0.9)
//    {
//        kNewAccounts = kNewAccounts * 2;
//    }
    int kNewAccounts = (int) ceil(log2(numberOfNodes))*2;

    // Initializing attackers: Setting edges between the attackers and maximum external degree
    HashTable<string, Node> newAccounts = initializeAttackers(kNewAccounts);

    // For each targeted node Wj, we choose a set Nj (contained in newAccounts), such that all Nj are distinct,
    // Once the sets are defined, we give each attacker a maximum external degree
    generateTargetedNodesSet(targetedNodes, newAccounts, numberOfNodes);

    // Adding arbitrary edges from H to G - H, so that each attacker node has exactly Di edges to G - H
    // And writing it to file - new accounts and it`s edges - sub graph H and edges connecting it to G

    addExtraEdges(newAccounts, nodes, targetedNodes);
    writeFile(outputFolderPath, newAccounts);

    // Cleaning Memory
    nodes = unordered_set<string>();

    // End of execution
    chrono::high_resolution_clock::time_point endTime = chrono::high_resolution_clock::now();
    chrono::duration<double> endTimeSpan              = chrono::duration_cast<chrono::duration<double> >(endTime - startTime);
//    printf("end: %lf secs\n", endTimeSpan.count());

    return 0;
}