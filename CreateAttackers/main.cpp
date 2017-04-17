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

    for (int k = 0; k < newAccounts.size(); k++)
    {
        string nodeLabel = "attacker_" + to_string(k);
        Node node = newAccounts.get(nodeLabel);
        if (nodes.size() < (node.getMaximumExternalDegree() - node.getExternalDegree()))
        {
            node.setMaximumExternalDegree(node.getExternalDegree() + nodes.size());
            for (unsigned i = 0; i < nodes.bucket_count(); ++i)
            {
                for (auto local_it = nodes.begin(i); local_it != nodes.end(i); ++local_it)
                {
                    node.addNeighbor(*local_it);
                    node.setExternalDegree(node.getExternalDegree() + 1);
                    node.setDegree(node.getDegree() + 1);
                }
            }
        }
        else
        {
            unordered_set<string> possibleNodes = nodes;
            while (node.getMaximumExternalDegree() != node.getExternalDegree())
            {
                auto random_it = next(std::begin(possibleNodes), rand() % possibleNodes.size());
                node.addNeighbor(*random_it); // any node inside nodes unordered_set
                node.setExternalDegree(node.getExternalDegree() + 1);
                node.setDegree(node.getDegree() + 1);
                possibleNodes.erase(*random_it);
            }

            // clearing memory
            possibleNodes = std::unordered_set<string>();
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

void generateTargetedNodesSet (vector<string> targetedNodes, HashTable<string, Node> &newAccounts, int numberOfNodes)
{
    bool couldGenerateSets = false;

    // we create an int array with size = number of attackers
    // We start with a "0000000" binary number. For each position, if it`s 1, then that attacker should be at the
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
            if (numberOfNodes > targetedNodes.size())
            {
                // We have nodes that aren`t targets
                // So All targets attackers subsets can have at least one attacker
                // Otherwise, we leave one target with no connections to attackers. And we identify it last
                binary_subsets[i] = 1;
            }
        }

        attackersArray[i] = newAccounts.get("attacker_" + to_string(i));
    }

    for (int i = 0; i < targetedNodes.size(); i++)
    {
        cout << i << endl;
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

    // Once all targeted nodes have a subset (of attackers), we generate a maximum external degree
    // for each attacker
    vector<pair<string, int>> attackersExternalDegree;
    for (int i = 0; i < numberOfAttackers; i++)
    {
        newAccounts.set("attacker_" + to_string(i), attackersArray[i]);
        Node node = attackersArray[i];
        pair<string, int> pair1 ("attacker_" + to_string(i), node.getExternalDegree());
        attackersExternalDegree.push_back(pair1);
    }

    delete[] binary_subsets;
    delete[] add_one_binary;
    delete[] attackersArray;

    // Sorting vector by the external degree
    sort(attackersExternalDegree.begin(), attackersExternalDegree.end(),[](const pair<string, int>& p1, const pair<string, int>& p2) {
        return p1.second < p2.second; });

    while (!couldGenerateSets)
    {
        // Generating maximum external degree distribution
        // 1 <= d0 <= log2(numberOfNodes) == newAccounts.size()
        int y = 3; // constant
        int d0 = (rand() % (2 + y) * newAccounts.size()) + 1; // random number between 1 and log2(numberOfNodes)
        int maxD = attackersExternalDegree[attackersExternalDegree.size() -
                                           1].second; // biggest empirical external degree
        int d1 = findD1(d0, maxD);
        //d1 = d1 * ((newAccounts.size() - 1) / d0); // d1 is O(log2(numberOfNodes), smaller than pow(log2(),2);

        default_random_engine generator;
        generator.seed(newAccounts.size());
        uniform_int_distribution<int> distribution(d0, d1);

        // Generate a list with n numbers (n = newAccounts.size()) between d0 and d1
        vector<int> maximumExternalDegreeEmpiricalDistribution;
        for (int i = 0; i < newAccounts.size(); i++) {
            maximumExternalDegreeEmpiricalDistribution.push_back(distribution(generator));
        }
        sort(maximumExternalDegreeEmpiricalDistribution.begin(), maximumExternalDegreeEmpiricalDistribution.end());

        cout << "maximum external degree: " << maximumExternalDegreeEmpiricalDistribution[numberOfAttackers - 1]
             << endl;

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
    //int          y            = 3; // should this constant be a param?
    int kNewAccounts = ceil(log2(numberOfNodes));
    //unsigned int kNewAccounts = (unsigned int) ((2 + y) * log2(numberOfNodes));
    cout << "number of attackers: " << kNewAccounts << endl;

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
    printf("end: %lf secs\n", endTimeSpan.count());

    return 0;
}