//
//  simdseqscan.h
//  dwarf
//
//  Created by David Broneske on 02.03.15.
//  Copyright (c) 2015 David Broneske. All rights reserved.
//

#ifndef dwarf_simdseqscan_h
#define dwarf_simdseqscan_h
#include "Store.h"
#include "Index.h"
#include "Util.h"
#include <vector>
#include <string.h>


class SIMDSeqScan : Index
{
    
private:
    
    vector<uint32_t*> *cols;
    uint8_t* result;
    
    
public:
    SIMDSeqScan(Store* s) : Index(s){
        
        cols = new vector<uint32_t*>(s->NUM_DIM);
        result = new uint8_t[(s->NUM_POINTS+7)/8];
    };
    
    ~SIMDSeqScan(){
       for(uint32_t i = 0; i < STORE->NUM_DIM; i++){
            free((*cols)[i]);
       }
       delete cols;
       delete result;
    }
    void buildIndex(){
        uint32_t* col;
        vector<uint32_t>*sample = new vector<uint32_t>(STORE->NUM_POINTS);
        
        for(uint32_t dim = 0; dim < STORE->NUM_DIM; dim++){
            
            void* temp;
            if(posix_memalign(&temp, 16, (STORE->NUM_POINTS+16)*sizeof(uint32_t))!=0){

                cout << "MemAlign failed! "<< endl;
                exit(-1);
            }
            col =(uint32_t*)temp;
            
            int32_t dummy;
            for (uint32_t point = 0; point < STORE->NUM_POINTS; point++) {
                dummy = STORE->getPoint(point)[dim];
                col[point]=dummy;
            }
            (*cols)[dim]=col;
        }
        delete sample;
    }
    
    
    uint32_t exactMatch(uint32_t* query){
        
        
        
        __m128i* simd_array = reinterpret_cast<__m128i*>((*cols)[0]);
        __m128i lowerSIMD = _mm_set1_epi32(query[0]);
        
        
        __m128i toComp; int32_t toAdd;
        uint32_t simdLine;
        for(simdLine = 0; simdLine < (STORE->NUM_POINTS+7)/4;simdLine+=8){
            
            toAdd=0;
            
            toComp=_mm_load_si128(&simd_array[simdLine]);
            toAdd |= _mm_movemask_ps((__m128)_mm_cmpeq_epi32(toComp,lowerSIMD));
            
            toComp=_mm_load_si128(&simd_array[simdLine+1]);
            toAdd |= (_mm_movemask_ps((__m128)_mm_cmpeq_epi32(toComp,lowerSIMD)) << 4);
            
            toComp=_mm_load_si128(&simd_array[simdLine+2]);
            toAdd |= (_mm_movemask_ps((__m128)_mm_cmpeq_epi32(toComp,lowerSIMD)) << 8);
            
            toComp=_mm_load_si128(&simd_array[simdLine+3]);
            toAdd |= (_mm_movemask_ps((__m128)_mm_cmpeq_epi32(toComp,lowerSIMD)) << 12);
            
            toComp=_mm_load_si128(&simd_array[simdLine+4]);
            toAdd |= (_mm_movemask_ps((__m128)_mm_cmpeq_epi32(toComp,lowerSIMD)) << 16);
            
            toComp=_mm_load_si128(&simd_array[simdLine+5]);
            toAdd |= (_mm_movemask_ps((__m128)_mm_cmpeq_epi32(toComp,lowerSIMD)) << 20);
            
            toComp=_mm_load_si128(&simd_array[simdLine+6]);
            toAdd |= (_mm_movemask_ps((__m128)_mm_cmpeq_epi32(toComp,lowerSIMD)) << 24);
            
            toComp=_mm_load_si128(&simd_array[simdLine+7]);
            toAdd |= (_mm_movemask_ps((__m128)_mm_cmpeq_epi32(toComp,lowerSIMD)) << 28);
            
            if(toAdd){
                for(int iter = 0; iter < 32;iter++){
                    if(isEqualCStore(simdLine*4+iter,query,STORE->NUM_DIM,cols))
                        return simdLine*4+iter;
                }
            }
        }
        for(uint32_t last = simdLine*4; last < (STORE->NUM_POINTS+7);last++){
            if(isEqualCStore(last,query,STORE->NUM_DIM,cols))
                return last;
        }
        return NOT_FOUND;
    }
    

    
    vector<uint32_t>* partialMatch(uint32_t* lowerBoundQuery, uint32_t* upperBoundQuery, bool* columnsForSelect){
        vector<uint32_t>* resultTIDs = new vector<uint32_t>();;
        memset(this->result,255,(STORE->NUM_POINTS+7)/8);
        uint32_t tid;
        
        for (uint32_t dim = 0; dim < STORE->NUM_DIM; dim++) {
            if(columnsForSelect[dim]){
                __m128i* simd_array = reinterpret_cast<__m128i*>((*cols)[dim]);
                __m128i lowerSIMD = _mm_set1_epi32(lowerBoundQuery[dim]-1);
                __m128i upperSIMD = _mm_set1_epi32(upperBoundQuery[dim]+1);
                
                for(tid = 0; tid+31 < ( STORE->NUM_POINTS+7);tid+=32){
                    
                    SIMDcheck32Tids_AND(&simd_array[tid/4], tid, this->result, lowerSIMD, upperSIMD);
                }
                for(;tid+15 < (STORE->NUM_POINTS+7);tid+=16){
                    SIMDcheckTids_AND(&simd_array[tid/4], tid, this->result, lowerSIMD, upperSIMD);
                }
            }
        }
        returnTids(resultTIDs, this->result,STORE->NUM_POINTS);
        
        
        return resultTIDs;
    }
    
    vector<uint32_t>* windowQuery(uint32_t* lowerBoundQuery, uint32_t* upperBoundQuery){
        vector<uint32_t>* resultTIDs = new vector<uint32_t>();;
        memset(this->result,255,(STORE->NUM_POINTS+7)/8);
        uint32_t tid;
        
        for (uint32_t dim = 0; dim < STORE->NUM_DIM; dim++) {
            
            __m128i* simd_array = reinterpret_cast<__m128i*>((*cols)[dim]);
            __m128i lowerSIMD = _mm_set1_epi32(lowerBoundQuery[dim]-1);
            __m128i upperSIMD = _mm_set1_epi32(upperBoundQuery[dim]+1);
            
            for(tid = 0; tid+31 < ( STORE->NUM_POINTS+7);tid+=32){
                
                SIMDcheck32Tids_AND(&simd_array[tid/4], tid, this->result, lowerSIMD, upperSIMD);
            }
            for(;tid+15 < (STORE->NUM_POINTS+7);tid+=16){
                SIMDcheckTids_AND(&simd_array[tid/4], tid, this->result, lowerSIMD, upperSIMD);
            }
        }
        returnTids(resultTIDs, this->result,STORE->NUM_POINTS);
        
        
        return resultTIDs;
    }
};

#endif
