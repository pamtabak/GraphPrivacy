#include <iostream>
#include "HashTable.hpp"
#include "Node.hpp"
#include <random>
#include <unordered_set>
#include <unordered_map>
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

// Reads input graph and returns its node`s labels and number of nodes
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

// Reads file and returns all targeted nodes labels
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
            // Adding the edge that closes the cycle connecting the attackers
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

    // the hashset nodes representes all nodes from the original graph that are not targets

    // each non target node can be only connected to one attacker
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
            d1 = ((((rand() % newAccounts.size()) + 1)*((rand() % newAccounts.size()) + 1))  + externalDegree);
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

// checks whether fist vector includes the second one
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

bool hasAutomorphism (HashTable<string, Node> newAccounts)
{
    // Each attacker`s degree appears once. If 2 or + attackers have the same degree
    // We check if the neighbor's degree sequence from one node (that contains only by nodes that are attackers) contains the other node's sequence
    unordered_map<int, vector<vector<int>>> degree;

    for (int k = 0; k < newAccounts.size(); k++)
    {
        string nodeLabel = "attacker_" + to_string(k);
        Node node = newAccounts.get(nodeLabel);

        list<string> neighbors = node.neighbors;
        vector<int> neighborsDegree;
        for (list<string>::iterator it=neighbors.begin(); it != neighbors.end(); ++it)
        {
            unordered_map<string, Node>::const_iterator got = newAccounts.hashMap.find(*it);
            if (got != newAccounts.hashMap.end())
            {
                // attacker
                Node neighborNode = got->second;
                neighborsDegree.push_back(neighborNode.getDegree());
            }
        }

        sort(neighborsDegree.begin(), neighborsDegree.end());

        unordered_map<int, vector<vector<int>>>::const_iterator got = degree.find(node.getDegree());
        if (got != degree.end())
        {
            // there is another attacker with the same degree
            for (int i = 0; i < got->second.size(); i++)
            {
                // compare neighbors degree with each already saved list of neighbors degrees
                if (got->second[i].size() > neighborsDegree.size())
                {
                    if (includes(got->second[i], neighborsDegree))
                    {
                        return true;
                    }
                }
                else
                {
                    if (includes(neighborsDegree, got->second[i]))
                    {
                        return true;
                    }
                }
            }
            degree[node.getDegree()].push_back(neighborsDegree);
        }
        else
        {
            degree[node.getDegree()].push_back(neighborsDegree);
        }
    }

    return false;
}

HashTable<string, Node> addAtackers (vector<string> targetedNodes, int kNewAccounts , int numberOfNodes, unordered_set<string> nodes)
{
    // Initializing attackers: Setting edges between the attackers and maximum external degree
    HashTable<string, Node> newAccounts;

    int  maxRetries      = 10;
    bool noAutomorphisms = false;

    while (!noAutomorphisms && maxRetries >= 0)
    {
        newAccounts = initializeAttackers(kNewAccounts);

        // we create an int array with size = number of attackers
        // We start with a "0000001" binary number. For each position, if it`s 1, then that attacker is going to
        // to be connected to that target. At each iteration, we add one to the binary number
        // This way, all targets will have different attacker subsets
        int numberOfAttackers = newAccounts.size();
        int *binary_subsets   = new int[numberOfAttackers];
        int *add_one_binary   = new int[numberOfAttackers];

        // Making access to nodes faster
        Node *attackersArray = new Node[numberOfAttackers];
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
            // targets must have, at least, 2 attackers (so we can connect non targets to attackers)
            if (onlyOneAttacker(binary_subsets, numberOfAttackers))
            {
                binarySum(add_one_binary, numberOfAttackers, binary_subsets);
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

            binarySum(add_one_binary, numberOfAttackers, binary_subsets);
        }

        for (int i = 0; i < newAccounts.size(); i++)
        {
            newAccounts.set("attacker_" + to_string(i), attackersArray[i]);
        }

        calculateMaximumExternalDegree(newAccounts);

        // Adding arbitrary edges from H to G - H, increasing the randomness of the attacker's subgraph
        addExtraEdges(newAccounts, nodes, targetedNodes);

        delete[] binary_subsets;
        delete[] add_one_binary;
        delete[] attackersArray;

        // Check for automorphisms
        // If two attackers have the same degree and same neighbor`s (that are attackers) degree sequence - if one sequence contains the other!
        // repeat process
        if (!hasAutomorphism(newAccounts))
        {
            noAutomorphisms = true;
        }
        else
        {
            maxRetries -= 1;
        }
    }
    return newAccounts;
}

int main (int argc, char * argv[])
{
    startTime = chrono::high_resolution_clock::now();

    // Output: attackers and it`s edges. Attacker's degrees and neighbor's degree sequence
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

    int kNewAccounts = (int) ceil(log2(numberOfNodes))*2;

    // For each targeted node Wj, we choose a set Nj (contained in newAccounts), such that all Nj are distinct,
    // Once the sets are defined, we give each attacker a maximum external degree
    HashTable<string, Node> newAccounts = addAtackers(targetedNodes, kNewAccounts, numberOfNodes, nodes);

    // writing it to file - new accounts and it`s edges - sub graph H and edges connecting it to G
    writeFile(outputFolderPath, newAccounts);

    // Cleaning Memory
    nodes = unordered_set<string>();

    // End of execution
    chrono::high_resolution_clock::time_point endTime = chrono::high_resolution_clock::now();
    chrono::duration<double> endTimeSpan              = chrono::duration_cast<chrono::duration<double> >(endTime - startTime);
//    printf("end: %lf secs\n", endTimeSpan.count());

    return 0;
}