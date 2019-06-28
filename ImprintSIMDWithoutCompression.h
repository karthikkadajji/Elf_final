//
//  ImprintSIMDWithoutCompression.h
//  dwarf
//
//  Created by David Broneske on 06.03.15.
//  Copyright (c) 2015 David Broneske. All rights reserved.
//

#ifndef dwarf_ImprintSIMDWithoutCompression_h
#define dwarf_ImprintSIMDWithoutCompression_h

#include "Imprint.h"
#include "Util.h"
#include <iostream>
#include <algorithm>
#include <string.h>

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



class ImprintSIMDIndexWithoutCompression : Index
{
private:
    vector<uint32_t*> *cols;
    uint8_t* result;
    vector<ColumnImprint*> columnImprints;
    const uint32_t NUM_CACHE_LINES;
    const uint32_t NUM_DIM;
    
    void createColumnImprint(uint32_t* column, uint32_t* sample, uint32_t curDim){
        ColumnImprint* imprints = columnImprints[curDim];
        
        void* temp;
        if(posix_memalign(&temp,16,200*sizeof(uint32_t))!=0){
            cout << "memalign failed!"<<endl;
            exit(-1);
        }
        imprints->bins=(uint32_t*)temp;
        
        if(posix_memalign(&temp,16,this->NUM_CACHE_LINES*sizeof(uint64_t))!=0){
            cout << "memalign failed!"<<endl;
            exit(-1);
        }
        imprints->imps=(uint64_t*)temp;
        
        //*************** Begin Fill Histogram *********************************//
        uint32_t k;
        double y, ystep = (double) STORE->NUM_POINTS / (64 - 1);
        for (k = 0, y = 0; (uint32_t) y < STORE->NUM_POINTS; y += ystep, k++){
            imprints->bins[k] = sample[(uint32_t) y];
        }
        if (k == 64 - 1){  //there is one left
            imprints->bins[k] = sample[STORE->NUM_POINTS - 1];
        }
        //*************** End Fill Histogram *********************************//
        
        
        //*************** Begin Create Imprints ******************************
        uint64_t mask = 0;
        uint32_t newCacheLine = (IMPS_PAGE/sizeof(int))-1;
        uint32_t impCount = 0;
        uint32_t bit_to_set = 0;
        
        for (uint32_t tid = 0; tid < STORE->NUM_POINTS; tid++) {
            if(!(tid&newCacheLine) && tid>0){//finished this cache line
                imprints->imps[impCount++] = mask;
                mask = 0;
            }
            
            bit_to_set=GETBIN64(column[tid], imprints->bins);
            mask = IMPSsetBit(64,mask,bit_to_set);
        }
        
        //The last chase line may not be full and is thus not added to the imps
        imprints->imps[impCount++] = mask;
        //assert(impCount==this->NUM_CACHE_LINES);
        //*************** End Create Imprints ******************************//
    }
    
    inline uint64_t make_mask(const ColumnImprint* imprints, const uint32_t lower, const uint32_t upper, uint64_t* tempInnermask){
        uint32_t lowerBitToSet=0, upperBitToSet=0;
        lowerBitToSet=GETBIN64( lower, imprints->bins);
        upperBitToSet=GETBIN64( upper, imprints->bins);
        uint64_t mask = (((((uint64_t) 1 << upperBitToSet) - 1) << 1) | 1) - (((uint64_t) 1 << lowerBitToSet) - 1);
        *tempInnermask = mask;
        if(lower != STORE->DIM_MIN)//und nicht imprints->bins[lowerBitToSet] == lower
            *tempInnermask = IMPSunsetBit(64, *tempInnermask, lowerBitToSet);
        if (upper != STORE->DIM_MAX)
            *tempInnermask = IMPSunsetBit(64, *tempInnermask, upperBitToSet);
        return mask;
    }
    
    uint8_t* range_select_column(const uint32_t curDim, const uint32_t lower, const uint32_t upper){
        uint8_t* result = this->result;
        uint32_t tid=0;
        uint32_t curCacheLine = 0;
        const ColumnImprint* imprints = this->columnImprints[curDim];
        const uint64_t* IMPRINT_VECTORS = imprints->imps;
        uint64_t tempInnermask = 0;
        const uint64_t mask = make_mask(imprints, lower, upper, &tempInnermask);
        const uint64_t innermask = ~tempInnermask;//we compute the complement only once, and this way i can use a const value
        uint64_t curImprint;
        
        __m128i* simd_array = reinterpret_cast<__m128i*>((*cols)[curDim]);
        __m128i lowerSIMD = _mm_set1_epi32(lower-1);
        __m128i upperSIMD = _mm_set1_epi32(upper+1);
        
//        __m128i SIMDmask = _mm_set1_epi64((__m64)mask);
//        __m128i SIMDInnermask = _mm_set1_epi64((__m64)innermask);
//        __m128i* SIMD_IMPRINT_VECTOR = reinterpret_cast<__m128i*>(imprints->imps);
//        __m128i curSIMDImprint;
//     
        for(;curCacheLine<NUM_CACHE_LINES;curCacheLine++){
            curImprint = IMPRINT_VECTORS[curCacheLine];
            tid = curCacheLine*16;
            //            cout << "huhu" << endl;
            if(curImprint&mask){//hit outer mask
                if ((curImprint&innermask)){//need to filter false positives
                    SIMDcheckTids_AND(&simd_array[tid/4],tid, result, lowerSIMD, upperSIMD);
                }
            }else{
//                count++;
                result[tid/8]=0;
                result[(tid/8)+1]=0;
            }
        }
        return result;
    }
    
    
    inline uint64_t make_mask_exact(uint32_t compareValue){
        uint32_t bitToSet = GETBIN64(compareValue, columnImprints[0]->bins);
        uint64_t mask=0;
        mask=IMPSsetBit(64,mask,bitToSet);
        return mask;
    }
    
public:
    ImprintSIMDIndexWithoutCompression(Store* s) : Index(s), NUM_CACHE_LINES((s->NUM_POINTS+15)/16), NUM_DIM(s->NUM_DIM){
        for (uint32_t dim = 0; dim < s->NUM_DIM; dim++) {
            ColumnImprint* dummy = new ColumnImprint();
            columnImprints.push_back(dummy);
        }
        cols = new vector<uint32_t*>(NUM_DIM);
        result = new uint8_t[(s->NUM_POINTS+7)/8];
    }
    
    void buildIndex(){
        uint32_t* col;
        vector<uint32_t>*sample = new vector<uint32_t>(STORE->NUM_POINTS);
        
        for(uint32_t dim = 0; dim < STORE->NUM_DIM; dim++){
            //#ifndef NO_SIMD_OPERATIONS
            //allocate columns
            void* temp;
            if(posix_memalign(&temp, IMPS_PAGE, (STORE->NUM_POINTS+16)*sizeof(uint32_t))!=0){

                cout << "MemAlign failed! "<< endl;
                exit(-1);
            }
            col =(uint32_t*)temp;
            //allocate result vectors, I want to allocate always a whole SIMD lane
            //                   posix_memalign(&temp, IMPS_PAGE, ((STORE->NUM_POINTS+15)/16)*2);
            //                   res= (uint8_t*)temp;
            //#else
            //                   col = new uint32_t[STORE->NUM_POINTS+16];//allocate columns
            ////                   uint32_t size = (STORE->NUM_POINTS+7)/8;
            ////                   res = new uint8_t[size];//allocate result vectors
            ////                   res[size-1] = 0;
            //#endif
            
            for (uint32_t point = 0; point < STORE->NUM_POINTS; point++) {
                uint32_t dummy = STORE->getPoint(point)[dim];
                col[point]=dummy;
                (*sample)[point]=dummy;
            }
            std::sort(sample->begin(), sample->end());
            (*cols)[dim]=col;
            createColumnImprint(col, &((*sample)[0]), dim);
        }
        delete sample;
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
    
    
    uint32_t exactMatch(uint32_t* query){
        uint64_t mask;
        mask = make_mask_exact(query[0]);
        uint64_t curImprint;
        const uint64_t* IMPRINT_VECTORS = this->columnImprints[0]->imps;
        uint32_t tid;
        
        
       // __m128i SIMDmask = _mm_set1_epi64((__m64)mask);
       // __m128i* SIMD_IMPRINT_VECTOR = reinterpret_cast<__m128i*>(this->columnImprints[0]->imps);
       // __m128i curSIMDImprint;
        uint32_t curCacheLine=0;
        
        
        for(;curCacheLine<NUM_CACHE_LINES;curCacheLine++){
            curImprint = IMPRINT_VECTORS[curCacheLine];
            
            if(curImprint&mask){//hit mask
                for(tid = curCacheLine*16;tid<(curCacheLine+1)*16;tid++){
                    if(isEqualCStore(tid, query, NUM_DIM)){
                        return tid;
                    }
                }
            }
        }
        
        return NOT_FOUND;
    }
    
    
    vector<uint32_t>* partialMatch(uint32_t* lowerBoundQuery, uint32_t* upperBoundQuery, bool* columnsForSelect){
        vector<uint32_t>* resultTIDs = new vector<uint32_t>();;
        memset(this->result,255,(STORE->NUM_POINTS+7)/8);
        
        for (uint32_t dim = 0; dim < STORE->NUM_DIM; dim++) {
            if(columnsForSelect[dim]){
                range_select_column(dim, lowerBoundQuery[dim], upperBoundQuery[dim]);
            }
        }
        returnTids(resultTIDs, this->result,STORE->NUM_POINTS);
        
        
        return resultTIDs;
    }
    
    
    vector<uint32_t>* windowQuery(uint32_t* lowerBoundQuery, uint32_t* upperBoundQuery){
        vector<uint32_t>* resultTIDs = new vector<uint32_t>();
        memset(this->result,255,(STORE->NUM_POINTS+7)/8);
        
        for (uint32_t dim = 0; dim < NUM_DIM; dim++) {
            range_select_column(dim, lowerBoundQuery[dim], upperBoundQuery[dim]);
        }
        returnTids(resultTIDs, this->result,STORE->NUM_POINTS);
        
        return resultTIDs;
    }
    
    ~ImprintSIMDIndexWithoutCompression(){
        for(uint32_t dim = 0; dim < NUM_DIM; dim++){
            free ((*cols)[dim]);
            //                   delete[] result;
            free(columnImprints[dim]->bins);
            free(columnImprints[dim]->imps);
            delete columnImprints[dim];
        }
        delete cols;
        delete[] result;
    }
    
//    int count=0;
};



#endif
