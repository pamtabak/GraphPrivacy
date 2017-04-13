//
// Created by Pamela Tabak on 13/04/17.
//

#ifndef RECOVERGRAPH_NODE_HPP
#define RECOVERGRAPH_NODE_HPP

#include <iostream>
#include <unordered_set>
#include <vector>

using namespace std;

class Node
{
public:
    int degree;
    vector<int> attackersNeighborsDegree;

    Node ()
    {
        this->degree = 0;
    }

    ~Node()
    {

    }

    int getDegree()
    {
        return this->degree;
    }

    void setDegree (int degree)
    {
        this->degree = degree;
    }

    void setAttackerNeighborDegree (vector<int> attackersNeighborDegree)
    {
        this->attackersNeighborsDegree = attackersNeighborDegree;
    }

    vector<int> getAttackerNeighborDegree ()
    {
        return this->attackersNeighborsDegree;
    }
};

#endif //RECOVERGRAPH_NODE_HPP
