//
//  colseqscan.h
//  dwarf
//
//  Created by David Broneske on 08.07.15.
//  Copyright (c) 2015 David Broneske. All rights reserved.
//

#ifndef dwarf_colseqscan_h
#define dwarf_colseqscan_h


#include <string.h>
class Colseqscan : Index
{
    
private:
    
    vector<uint32_t*> *cols;
    uint8_t* result;
    
    
public:
    Colseqscan(Store* s) : Index(s){
        
        cols = new vector<uint32_t*>(s->NUM_DIM);
        result = new uint8_t[(s->NUM_POINTS+15)/8];
    };
    
    ~Colseqscan(){
        for(uint32_t i = 0; i < STORE->NUM_DIM; i++){
            free((*cols)[i]);
        }
        delete cols;
        delete result;
    }
    void buildIndex(){
        uint32_t* col;
        
        for(uint32_t dim = 0; dim < STORE->NUM_DIM; dim++){
            
            col =(uint32_t*)malloc((STORE->NUM_POINTS+16)*sizeof(uint32_t));
            if(!col){
                
                cout << "Malloc failed! "<< endl;
                exit(-1);
            }
            
            int32_t dummy;
            for (uint32_t point = 0; point < STORE->NUM_POINTS; point++) {
                dummy = STORE->getPoint(point)[dim];
                col[point]=dummy;
            }
            (*cols)[dim]=col;
        }
    }
    
    
    uint32_t exactMatch(uint32_t* query){
        
        
        uint32_t* col=(*cols)[0];
        uint32_t toComp=query[0];
        for(uint32_t tid = 0; tid < STORE->NUM_POINTS;tid++){
            
            if(col[tid] == toComp){
                if(isEqualCStore(tid,query,STORE->NUM_DIM,cols))
                    return tid;
            }
        }
        return NOT_FOUND;
    }
    
    
    
    vector<uint32_t>* partialMatch(uint32_t* lowerBoundQuery, uint32_t* upperBoundQuery, bool* columnsForSelect){
        vector<uint32_t>* resultTIDs = new vector<uint32_t>();;
        memset(this->result,255,(STORE->NUM_POINTS+7)/8);
        uint32_t tid;
        
        for (uint32_t dim = 0; dim < STORE->NUM_DIM; dim++) {
            if(columnsForSelect[dim]){
                
                for(tid = 0; tid < ( STORE->NUM_POINTS);tid+=16){
                    
                    checkTids_AND((*cols)[dim], tid, this->result, lowerBoundQuery[dim], upperBoundQuery[dim]);
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
            for(tid = 0; tid < ( STORE->NUM_POINTS);tid+=16){
                checkTids_AND((*cols)[dim], tid, this->result, lowerBoundQuery[dim], upperBoundQuery[dim]);
            }
        }
        returnTids(resultTIDs, this->result,STORE->NUM_POINTS);
        
        
        return resultTIDs;
    }
};

#endif
