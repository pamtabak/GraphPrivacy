//
// Created by Pamela Tabak on 08/02/17.
//

#ifndef CREATEATTACKERS_HASHTABLE_HPP
#define CREATEATTACKERS_HASHTABLE_HPP

#include <unordered_map>
#include <fstream>

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


#endif //CREATEATTACKERS_HASHTABLE_HPP
