#ifndef MEMORYSTORE_LEXICALCAST_H
#define MEMORYSTORE_LEXICALCAST_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
using namespace std;

template<typename Result,typename Para>
Result lexical_cast(Para para)
{
    stringstream ss;
    ss<<para;
    Result result;
    ss>>result;
    return result;
}

#endif