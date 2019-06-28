/*
 * The contents of this file are subject to the MonetDB Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.monetdb.org/Legal/MonetDBLicense
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is the MonetDB Database System.
 *
 * The Initial Developer of the Original Code is CWI.
 * Portions created by CWI are Copyright (C) 1997-July 2008 CWI.
 * Copyright August 2008-2015 MonetDB B.V.
 * All Rights Reserved.
 */

/*
 * Implementation for the column imprints index.
 * See paper:
 * Column Imprints: A Secondary Index Structure,
 * L.Sidirourgos and M.Kersten.
 */
//  Imprint.cpp
//  dwarf
//
//  Created by David Broneske on 25.01.15.
//  Copyright (c) 2015 David Broneske. All rights reserved.
//

#include "Imprint.h"
#include <limits.h>
#include <algorithm>    // std::sort
#include <string.h>
#include "stdlib.h"

//#define GETBIN64(Z,X)

// SSE compiler intrinsics
#ifdef __SSE__
#include <xmmintrin.h>
#endif

//For SSE2:
#ifdef __SSE2__
extern "C"
{
#include <emmintrin.h>
#include <mmintrin.h>
}
#endif


//For SSE3:
#ifdef __SSE3__
extern "C"
{
#include <pmmintrin.h>
#include <immintrin.h>   // (Meta-header, for GCC only)
}
#endif

//For SSE4: (WITHOUT extern "C")
#ifdef __SSE4_1__
#include <smmintrin.h>
#endif


//For SSE4: (WITHOUT extern "C")
#ifdef __SSE4_2__
#include <nmmintrin.h>
#endif

#ifdef __AVX__
#include "immintrin.h"
//#include "avxintrin.h"
#endif

uint8_t* ImprintIndex::select_column(int curDim, ImprintPerColumn* imprints, uint32_t compareValue){
	uint32_t i_cnt = 0;
	uint32_t cache_cnt = 0;
	uint32_t tid_cnt = 0;

	uint8_t* res = (*results)[curDim];
	uint64_t mask;
	mask = make_mask_exact(imprints, compareValue);

	//for each line in the dictionary
	for(uint32_t i = 0; i < imprints->dictcnt; i++){
		//not a repeat header
		if(imprints->dict[i].repeat==0){
			uint32_t numImprintsInThisDictLine = imprints->dict[i].cnt;
			uint32_t stoppOffset = i_cnt + numImprintsInThisDictLine -1;
			for (uint32_t j = i_cnt; j <= stoppOffset; j++) {
				if (imprints->imps[j]&mask) {//außere Maske getroffen
					checkTids_OR((*cols)[curDim],cache_cnt*16,res, compareValue, compareValue);//FIXME bissl doppelt
				}
				cache_cnt++;
			}
			i_cnt+=imprints->dict[i].cnt;

		}else{//Repeat gesetzt
			if(imprints->imps[i_cnt]&mask){
				for(tid_cnt = 16*cache_cnt; tid_cnt < cache_cnt*16+(16*imprints->dict[i].cnt-1); tid_cnt+=16){
					checkTids_OR((*cols)[curDim],tid_cnt,res, compareValue, compareValue);//FIXME bissl doppelt
				}
			}
			i_cnt++;
			cache_cnt+=imprints->dict[i].cnt;
		}
	}
	return res;
}

vector<uint32_t>* ImprintIndex::partialMatch(uint32_t* lowerBoundQuery, uint32_t* upperBoundQuery, bool* columnsForSelect){
    vector<uint32_t>* resultTIDs = new vector<uint32_t>();
    uint32_t dim = 0;
    while (!columnsForSelect[dim]) {
        dim++;
    }
    uint8_t* res1 = range_select_column(dim, imps[dim], lowerBoundQuery[dim], upperBoundQuery[dim]);
    uint8_t* res2;
    for (; dim < STORE->NUM_DIM; dim++) {
        if(columnsForSelect[dim]){
            res2 = range_select_column(dim, imps[dim], lowerBoundQuery[dim], upperBoundQuery[dim]);
            andBitvectors( res1, res2, STORE->NUM_POINTS);
        }
    }
    returnTids(resultTIDs, res1,STORE->NUM_POINTS);
    
    
    return resultTIDs;
}

vector<uint32_t>* ImprintIndex::windowQuery( uint32_t* lowerBoundQuery,  uint32_t* upperBoundQuery){
	vector<uint32_t>* resultTIDs = new vector<uint32_t>();

	uint8_t* res1 = range_select_column(FIRST_DIM, imps[FIRST_DIM], lowerBoundQuery[FIRST_DIM], upperBoundQuery[FIRST_DIM]);
	uint8_t* res2;
	for (uint32_t dim = 1; dim < STORE->NUM_DIM; dim++) {
		res2 = range_select_column(dim, imps[dim], lowerBoundQuery[dim], upperBoundQuery[dim]);
		andBitvectors( res1, res2, STORE->NUM_POINTS);
	}
	returnTids(resultTIDs, res1,STORE->NUM_POINTS);

	return resultTIDs;
}

void printResultBM(uint8_t* res,uint64_t numEntries){
	for (uint32_t i = 0; i < numEntries; i++) {
		cout << hex << (int)res[i] <<endl;
	}
}

uint8_t* ImprintIndex::range_select_column(uint32_t curDim, ImprintPerColumn* imprints, uint32_t lower, uint32_t upper){
	uint32_t i_cnt = 0;
	uint32_t cache_cnt = 0;
	uint32_t tid_cnt = 0;

	uint8_t* res = (*results)[curDim];
	memset(res,0,(STORE->NUM_POINTS+7)/8);

	uint64_t mask;
	uint64_t innermask;
	make_mask(imprints, &mask, &innermask, lower, upper);

	//for each line in the dictionary
	for(uint32_t i = 0; i < imprints->dictcnt; i++){
		//not a repeat header
		if(imprints->dict[i].repeat==0){
			uint32_t numImprintsInThisDictLine = imprints->dict[i].cnt;
			uint32_t stoppOffset = i_cnt + numImprintsInThisDictLine -1;
			for (uint32_t j = i_cnt; j <= stoppOffset; j++) {
				if (imprints->imps[j]&mask) {//außere Maske getroffen
					if (imprints->imps[j]& ~innermask) {//inner Maske nicht gematcht -> False positives rausfiltern!
                        checkTids_OR((*cols)[curDim],cache_cnt*16,res, lower, upper);
                    }else{//matches ONLY inner mask
                        addTidsToResult(cache_cnt*16,res);
					}
				}
				cache_cnt++;
			}
			i_cnt+=imprints->dict[i].cnt;

		}else{//Repeat gesetzt
			if(imprints->imps[i_cnt]&mask){
                if((imprints->imps[i_cnt]&~innermask)){
                    for(tid_cnt = 16*cache_cnt; tid_cnt < cache_cnt*16+(16*imprints->dict[i].cnt-1); tid_cnt+=16){
                        checkTids_OR((*cols)[curDim],tid_cnt,res, lower, upper);
                    }
				}else{
                    for(tid_cnt = 16*cache_cnt; tid_cnt < cache_cnt*16+(16*imprints->dict[i].cnt-1); tid_cnt+=16){
                        addTidsToResult(tid_cnt, res);
                    }
                }
			}
			i_cnt++;
			cache_cnt+=imprints->dict[i].cnt;
		}


	}
//    printResultBM(res, STORE->NUM_POINTS/8);
	return res;

}


	/* construct the mask */
	inline void
	ImprintIndex::make_mask(ImprintPerColumn* imprints, uint64_t* mask, uint64_t* innermask, uint32_t lower, uint32_t upper){
		uint32_t lowerBitToSet=0, upperBitToSet=0;
		lowerBitToSet=GETBIN64( lower, imprints->bins);
		upperBitToSet=GETBIN64( upper, imprints->bins);
		*mask = (((((uint64_t) 1 << upperBitToSet) - 1) << 1) | 1) - (((uint64_t) 1 << lowerBitToSet) - 1); //XXX To be tested for functioning
		*innermask = *mask;
		if(lower != STORE->DIM_MIN)//und nicht imprints->bins[lowerBitToSet] == lower
			*innermask = IMPSunsetBit(64, *innermask, lowerBitToSet);
		if (upper != STORE->DIM_MAX)
			*innermask = IMPSunsetBit(64, *innermask, upperBitToSet);

	}

	int
	ImprintIndex::imprints_create(uint32_t *b,ImprintPerColumn* imprints)
	{
//        BUN i;
		BUN dcnt, icnt, newc;
		uint8_t bit_to_set = 0;
		dcnt = icnt = 0;

		uint64_t mask, prvmask;
		uint64_t * im = (uint64_t *) imprints->imps;
		const int * col = (int *) b;
//		const int * bins = (int *) imprints->bins;
//		int nil = UINT_MAX;
		prvmask = mask = 0;
		newc = (IMPS_PAGE/sizeof(int))-1;
		for (uint32_t i = 0; i < STORE->NUM_POINTS; i++) {
			if (!(i&newc) && i>0) {
				/* same mask as previous and enough count to add */
//				cout << i << endl;
//				cout << prvmask<< " -- "<< mask << endl;
				if ((prvmask == mask) &&
					(imprints->dict[dcnt-1].cnt < (IMPS_MAX_CNT-1))) {
					/* not a repeat header */
					if (!imprints->dict[dcnt-1].repeat) {
						/* if compressed */
						if (imprints->dict[dcnt-1].cnt > 1) {
							/* uncompress last */
							imprints->dict[dcnt-1].cnt--;
							dcnt++; /* new header */
							imprints->dict[dcnt-1].cnt = 1;
						}
						/* set repeat */
						imprints->dict[dcnt-1].repeat = 1;
					}
					/* increase cnt */
					imprints->dict[dcnt-1].cnt++;
				} else { /* new mask (or run out of header count) */
					prvmask=mask;
					im[icnt] = mask;
					icnt++;
					if ((dcnt > 0) && !(imprints->dict[dcnt-1].repeat)) {//&& (imprints->dict[dcnt-1].cnt < (IMPS_MAX_CNT-1))
						imprints->dict[dcnt-1].cnt++;
					} else {
						imprints->dict[dcnt].cnt = 1;
						imprints->dict[dcnt].repeat = 0;
						imprints->dict[dcnt].flags = 0;
						dcnt++;
					}
				}
				/* new mask */
				//cout <<"new mask lines in dict: "<< dcnt << " referring to " << imprints->dict[dcnt-1].cnt << " lines of mask  " << mask << endl;
				mask = 0;
			}
//			cout << col[i] << "..." << i << endl;
			bit_to_set=GETBIN64(col[i], imprints->bins);
			mask = IMPSsetBit(64,mask,bit_to_set);
			//cout << col[i] << "..." << (uint32_t)bit_to_set << " - "<< mask << endl;

//            cout << (uint64_t)mask << endl;
		}
		/* one last left */
		if (prvmask == mask && dcnt > 0 &&
			(imprints->dict[dcnt-1].cnt < (IMPS_MAX_CNT-1))) {
			if (!imprints->dict[dcnt-1].repeat) {
				if (imprints->dict[dcnt-1].cnt > 1) {
					imprints->dict[dcnt-1].cnt--;
					imprints->dict[dcnt].cnt = 1;
					imprints->dict[dcnt].flags = 0;
					dcnt++;
				}
				imprints->dict[dcnt-1].repeat = 1;
			}
			imprints->dict[dcnt-1].cnt ++;
		} else {
			im[icnt] = mask;
			icnt++;
			if ((dcnt > 0) && !(imprints->dict[dcnt-1].repeat)) {//&& (imprints->dict[dcnt-1].cnt < (IMPS_MAX_CNT-1)
				imprints->dict[dcnt-1].cnt++;
				//cout <<"new mask lines in dict: "<< dcnt << " referring to " << imprints->dict[dcnt-1].cnt << " lines of mask  " << mask << endl;
			} else {
				imprints->dict[dcnt].cnt = 1;
				imprints->dict[dcnt].repeat = 0;
				imprints->dict[dcnt].flags = 0;
				dcnt++;
			}
		}



//        cout << dcnt << endl;
		imprints->dictcnt = dcnt;
		imprints->impcnt = icnt;

		return 1;
	}


	void ImprintIndex::buildIndex(){
		uint32_t* col;
		uint8_t* res;
		for(uint32_t dim = 0; dim < STORE->NUM_DIM; dim++){
#ifndef NO_SIMD_OPERATIONS
			//allocate columns
			void* temp;
		        if(posix_memalign(&temp, IMPS_PAGE, (STORE->NUM_POINTS+16)*sizeof(uint32_t))!=0){
			   cout << "memalign failed!"<<endl;
                           exit(1);
		        }
			col =(uint32_t*)temp;

			//allocate result vectors
			// i want to allocate always a whole SIMD lane

		        temp=NULL;
			//@David XXX raff ich nicht, Martin
	if(posix_memalign(&temp, IMPS_PAGE, ((STORE->NUM_POINTS+15)/8))!=0){
			cout << "memalign failed!"<<endl;
                         exit(1);
		}
		
			res= (uint8_t*)temp;
			// ich hab Angst, dass da hinten Kacke steht
//			memset(res+(STORE->NUM_POINTS-1), 0, 16);
#else
			//allocate columns
			col = new uint32_t[STORE->NUM_POINTS+16];
			//allocate result vectors
			uint32_t size = (STORE->NUM_POINTS+7)/8;
			res = new uint8_t[size];
			res[size-1] = 0;//könnte vor jede anfrage müssen
#endif
			vector<uint32_t>* sample = new vector<uint32_t>();

			for (uint32_t point = 0; point < STORE->NUM_POINTS; point++) {
				uint32_t dummy = STORE->getPoint(point)[dim];
				col[point]=dummy;
				sample->push_back(dummy);

			}
			sort(sample->begin(), sample->end());
			(*cols)[dim]=col;
			(*results)[dim]=res;
			BATimprints(col,&((*sample)[0]),imps[dim]);
			delete sample;
		}

	}


	int
	ImprintIndex::BATimprints(uint32_t* column, uint32_t* sample, ImprintPerColumn* imprints)
	{
#ifdef NO_SIMD_OPERATIONS
			imprints->dict = (cchdc_t*)  malloc(STORE->NUM_POINTS*sizeof(cchdc_t));
			imprints->bins = (uint32_t*) malloc(64*sizeof(uint32_t));
			imprints->imps = (uint64_t*) malloc(STORE->NUM_POINTS*sizeof(uint64_t));
#else
		void* temp;
		if(posix_memalign(&temp,16,STORE->NUM_POINTS*sizeof(cchdc_t))!=0){
			cout << "memalign failed!"<<endl;
		}
		imprints->dict=(cchdc_t*)temp;
//        cout <<"temp "<< temp << endl;
//        cout <<"dict "<< imprints->dict << endl;

		temp=NULL;
		if(posix_memalign(&temp,16,64*sizeof(uint32_t))!=0){
			cout << "memalign failed!"<<endl;
		}
		imprints->bins=(uint32_t*)temp;
//        cout <<"temp "<< temp << endl;
//        cout <<"bins "<< imprints->bins << endl;

		temp=NULL;
		if(posix_memalign(&temp,16,STORE->NUM_POINTS*sizeof(uint64_t))!=0){
			cout << "memalign failed!"<<endl;
		}
		imprints->imps=(uint64_t*)temp;
//        cout <<"temp "<< temp << endl;
//        cout <<"imps "<< imprints->imps << endl;
#endif

		uint32_t k;
		double y, ystep = (double) STORE->NUM_POINTS / (64 - 1);
		for (k = 0, y = 0; (uint32_t) y < STORE->NUM_POINTS; y += ystep, k++){
			imprints->bins[k] = sample[(uint32_t) y];
		}
		if (k == 64 - 1)  //there is one left
			imprints->bins[k] = sample[STORE->NUM_POINTS - 1];

//            for(int i=0;i< 64; i++){
//                cout <<imprints->bins[i] <<endl;
//            }


		if (!imprints_create(column,imprints)) {
			cout << "Error at imps_create"<<endl;
			exit(-1);
		}


		return 1;
	}
