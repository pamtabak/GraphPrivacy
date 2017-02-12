//
// Created by Pamela Tabak on 05/02/17.
//

#include <unordered_map>
#include <fstream>

#ifndef RANDOMPERMUTATION_HASHTABLE_HPP
#define RANDOMPERMUTATION_HASHTABLE_HPP

using namespace std;

template <typename K, typename T>
class HashTable
{
public:
    unordered_map<K, T> hashMap;

    void set(K key, T value)
    {
        this->hashMap[key] = value;
    }

    T get(K key)
    {
        return this->hashMap[key];
    }

    void writeToFile (string fileName)
    {
        ofstream file;
        file.open(fileName, fstream::app);
        for (unsigned i = 0; i < this->hashMap.bucket_count(); ++i)
        {
            for (auto local_it = this->hashMap.begin(i); local_it!= this->hashMap.end(i); ++local_it)
            {
                file << local_it->first << ":" << local_it->second << "\n";
            }
        }
        file.close();
    }
};

#endif //RANDOMPERMUTATION_HASHTABLE_HPP