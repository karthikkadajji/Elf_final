#ifndef SEQSCAN_H
#define SEQSCAN_H

#include "Store.h"
#include "Index.h"
#include "Util.h"
#include <vector>

class SeqScan : Index
{
	public:
		SeqScan(Store* s) : Index(s){	}

		uint32_t exactMatch(uint32_t* query){
			for(uint32_t tid = 0;tid<STORE->NUM_POINTS;tid++){
				if(isEqual(STORE->getPoint(tid), query, STORE->NUM_DIM)){
					return tid;
				}
			}
			return NOT_FOUND;
		}

		vector<uint32_t>* windowQuery(uint32_t* lowerBoundQuery, uint32_t* upperBoundQuery){
			vector<uint32_t>* resultTIDs = new vector<uint32_t>();
			uint32_t* point;

			for(uint32_t tid = 0; tid<STORE->NUM_POINTS;tid++){
				point = STORE->getPoint(tid);
				if (isIn(point, lowerBoundQuery, upperBoundQuery, STORE->NUM_DIM))
					resultTIDs->push_back(tid);
			}

			return resultTIDs;
		}

		vector<uint32_t>* partialMatch(uint32_t* lowerBoundQuery, uint32_t* upperBoundQuery, bool* columnsForSelect){
			vector<uint32_t>* resultTIDS = new vector<uint32_t>();
			uint32_t* point;

			for(uint32_t tid = 0; tid<STORE->NUM_POINTS;tid++){
				point = STORE->getPoint(tid);
				if (isPartiallyIn(point, lowerBoundQuery, upperBoundQuery, columnsForSelect, STORE->NUM_DIM))
					resultTIDS->push_back(tid);
			}

			return resultTIDS;
		}
};

#endif // SEQSCAN_H
