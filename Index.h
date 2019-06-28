#ifndef INDEX_H
#define INDEX_H

#include <vector>
#include <cstddef>
#include <iostream>
#include <stdint.h>
#include "Store.h"
//#include "ThreadPool.h"



using namespace std;
class Index
{
	public:
		Index(Store* s) : STORE(s){	}
		virtual void buildIndex(){}
		virtual uint32_t* getElf(){ return 0;}
		virtual uint32_t exactMatch(uint32_t* query){return -1;}
        virtual vector<uint32_t>* windowQuery(uint32_t* lowerBoundQuery, uint32_t* upperBoundQuery){
			vector<uint32_t>* emptyResult = new vector<uint32_t>();
			return emptyResult;
        }
//        virtual ThreadPool initialize_pool(int thread_num){}
		virtual vector<uint32_t>* partialMatch(uint32_t* lowerBoundQuery, uint32_t* upperBoundQuery, bool* columnsForSelect){
			vector<uint32_t>* emptyResult = new vector<uint32_t>();
			return emptyResult;
		}
		virtual void setNumThreads(int num_of_threads){};
    virtual void printSize(){};
		virtual void dowork(){};
	virtual ~Index(){};
		Store* STORE;
};

#endif // INDEX_H
