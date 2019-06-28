//
//  Imprint.h
//  dwarf
//
//  Created by David Broneske on 25.01.15.
//  Copyright (c) 2015 David Broneske. All rights reserved.
//

#ifndef __dwarf__Imprint__
#define __dwarf__Imprint__

/* hard bounds */
#define IMPS_MAX_CNT	((1 << 24) - 1)		/* 24 one bits */
#define IMPS_PAGE	64

/* auxiliary macros */
#define IMPSsetBit(B, X, Y)	((X) | ((uint##B##_t) 1 << (Y)))
#define IMPSunsetBit(B, X, Y)	((X) & ~((uint##B##_t) 1 << (Y)))
#define IMPSisSet(B, X, Y)	(((X) & ((uint##B##_t) 1 << (Y))) != 0)


#include <stdio.h>
#include "Index.h"
#include <stdint.h>
#include "Util.h"

typedef size_t BUN;
typedef uint32_t* BAT;
typedef signed char bte;




/*
 * the cache dictionary struct
 */
typedef struct {
	unsigned int cnt:24,   /* cnt of pages <= IMPS_MAX_CNT */
repeat:1, /* repeat flag                 */
flags:7;  /* reserved flags for future   */
} cchdc_t;


struct ImprintPerColumn {
	bte bits;		/* how many bits in imprints */
	//    uint32_t *imprints;
	uint32_t *bins;		/* pointer into imprints heap (bins borders)  */
	//BUN *stats;		/* pointer into imprints heap (stats per bin) */
	uint64_t *imps;		/* pointer into imprints heap (bit vectors)   */
	cchdc_t *dict;		/* pointer into imprints heap (dictionary)    */
	size_t impcnt;		/* counter for imprints                       */
	size_t dictcnt;		/* counter for cache dictionary               */
};

class ImprintIndex : public Index{
public:
    
    int numRepeats(int dim){
        ImprintPerColumn* current = imps[dim];
        int count=0;
		for(uint32_t impCount = 0; impCount < current->impcnt; impCount++ ){
            if(current->dict[impCount].repeat)
                count++;
        }
        return count;
    }

    
	inline bool isEqualCStore(const int TID, uint32_t* query, const uint32_t LENGTH){
		uint32_t value;
		uint32_t* column;

		for(uint32_t dim=FIRST_DIM;dim<LENGTH;dim++){
			column = (*cols)[dim];
			value = column[TID];
			if(value!=query[dim]){
				return false;
			}
		}
		return true;
	}
	inline void make_mask(ImprintPerColumn* imprints, uint64_t* mask, uint64_t* innermask, uint32_t lower, uint32_t upper);

	inline uint64_t
	make_mask_exact(ImprintPerColumn* imprints,  uint32_t compareValue){
		uint32_t bitToSet = GETBIN64(compareValue, imprints->bins);
		uint64_t mask=IMPSsetBit(64,0,bitToSet);
		return mask;
	}

	uint8_t* select_column(int dim, ImprintPerColumn* imprints, uint32_t compareValue);
	uint8_t* range_select_column(uint32_t curDim,ImprintPerColumn* imprints, uint32_t lower, uint32_t upper);

	int BATimprints(uint32_t* column,uint32_t* sample, ImprintPerColumn* imprints );
	vector<ImprintPerColumn*> imps;
	vector<uint32_t*>*cols;
	vector<uint8_t*>*results;
	//ImprintPerColumn** imps;

	ImprintIndex(Store* s) : Index(s){
		cols = new vector<uint32_t*>(s->NUM_DIM);
		results = new vector<uint8_t*>(s->NUM_DIM);

		for (uint32_t dim = 0; dim < s->NUM_DIM; dim++) {
			ImprintPerColumn* dummy = new ImprintPerColumn();
			imps.push_back(dummy);
		}
	}
	void buildIndex();
	int imprints_create(uint32_t *b,ImprintPerColumn* imprints);

	uint32_t exactMatch(uint32_t* query){
		ImprintPerColumn* IMPRINT_ON_FIRST_COLUMN = this->imps[FIRST_DIM];
		uint64_t* BIT_VECTORS = IMPRINT_ON_FIRST_COLUMN->imps;
		cchdc_t* DICTIONARY = IMPRINT_ON_FIRST_COLUMN->dict;

		const uint64_t mask = make_mask_exact(IMPRINT_ON_FIRST_COLUMN, query[FIRST_DIM]);

		uint32_t numImprintsProcessed = 0;
		uint32_t currentDictionaryLine = 0;
		uint64_t currentImprint = -1;
        uint32_t skippedCacheLines=0;
		const uint32_t LENGTH = STORE->NUM_DIM;

		//const uint32_t NUM_IMPRINTS = IMPRINT_ON_FIRST_COLUMN->impcnt;
		const uint32_t NUM_DICT_LINES = IMPRINT_ON_FIRST_COLUMN->dictcnt;

		for(;currentDictionaryLine<NUM_DICT_LINES;currentDictionaryLine++){
			if(DICTIONARY[currentDictionaryLine].repeat==0){//if not a repeat header
				uint32_t numUnCompressedImprints = DICTIONARY[currentDictionaryLine].cnt;//how many uncompressed adjacent imprints?
				for(uint32_t line=0;line<numUnCompressedImprints;line++){
					currentImprint = BIT_VECTORS[numImprintsProcessed];

					if(currentImprint&mask){//mask hit
						for(uint32_t tid = (numImprintsProcessed+skippedCacheLines)*16;tid<(numImprintsProcessed+skippedCacheLines+1)*16;tid++){
							if(isEqualCStore(tid,query,LENGTH)){
								return tid;
							}
						}
					}
					numImprintsProcessed++;
				}
			}else{
				uint32_t numCompressedImprints = DICTIONARY[currentDictionaryLine].cnt;//how many *compressed* adjacent imprints?
				currentImprint = BIT_VECTORS[numImprintsProcessed];
				if(currentImprint&mask){//only need to enter for loop if mask matches
					for(uint32_t tid=(numImprintsProcessed+skippedCacheLines)*16;tid<(numImprintsProcessed+numCompressedImprints+skippedCacheLines)*16;tid++){
						if(isEqualCStore(tid,query,LENGTH)){
							return tid;
						}
					}
				}
                skippedCacheLines+=numCompressedImprints;
                numImprintsProcessed++;//don't forget to increase numImprintsProcessed
			}
		}
		return NOT_FOUND;
	}

	vector<uint32_t>* windowQuery( uint32_t* lowerBoundQuery,  uint32_t* upperBoundQuery);
    
    vector<uint32_t>* partialMatch(uint32_t* lowerBoundQuery, uint32_t* upperBoundQuery, bool* columnsForSelect);

	~ImprintIndex(){
		for(uint32_t dim = 0; dim < STORE->NUM_DIM; dim++){
			free ((*cols)[dim]);
			free ((*results)[dim]);
			free(imps[dim]->bins);
			free(imps[dim]->imps);
			free(imps[dim]->dict);
			delete imps[dim];
		}
		delete cols;
	}
    
};

#endif /* defined(__dwarf__Imprint__) */
