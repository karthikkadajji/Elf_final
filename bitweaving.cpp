//
//  bitweaving.cpp
//  dwarf
//
//  Created by David Broneske on 13.01.15.
//  Copyright (c) 2015 David Broneske. All rights reserved.
//

#include "bitweaving.h"

using namespace bitweaving;

void BitWeaving::buildIndex(){
    
    
    
    // Create a table
    Options options = Options();
    options.in_memory = true;
    table = new Table("", options);
    table->Open();
    
    
    // Add two columns into the table
    for(uint32_t i = 0; i < STORE->NUM_DIM; i++){
        table->AddColumn(std::to_string(i), ctype, code_sizes[i]);
    }
    
    
    // Insert values into columns
    bitweaving::Column *column1;
    bitweaving::Code *codes1 = new bitweaving::Code[STORE->NUM_POINTS];
    for(uint32_t i = 0; i < STORE->NUM_DIM; i++){
        column1 = table->GetColumn(std::to_string(i));
        assert(column1 != NULL);
        for (uint32_t j = 0; j < STORE->NUM_POINTS; j++) {
            codes1[j]=STORE->getPoint(j)[i];
        }
        
        
        column1->Bulkload(codes1, STORE->NUM_POINTS);
    }
    
    
    
    
}




uint32_t BitWeaving::exactMatch(uint32_t* query){
    bitweaving::Column *column1;
    bitweaving::BitVector *bitvector = new bitweaving::BitVector(*table);
    
    column1 = table->GetColumn(std::to_string(0));
    column1->Scan(query[0], kEqual, bitvector);
    
  /*  for(uint32_t dim = 1;dim<STORE->NUM_DIM;dim++){
        column1 = table->GetColumn(std::to_string(dim));
        column1->Scan(query[dim], kEqual,kAnd, bitvector);
    }
    */
    
    
    // Create an iterator to get matching values
    bitweaving::Iterator *iter = new bitweaving::Iterator(*bitvector);
    
        while (iter->Next()) {
            uint32_t tid =iter->GetPosition();
            if(isEqual(STORE->getPoint(tid), query, STORE->NUM_DIM)){
                delete iter;
                delete bitvector;
                return tid;
            }

        }
        return NOT_FOUND;
    
    
}

vector<uint32_t>* BitWeaving::windowQuery(uint32_t* lowerBoundQuery, uint32_t* upperBoundQuery){
    vector<uint32_t>* resultTIDs = new vector<uint32_t>();;
    
    
    bitweaving::Column *column1;
    bitweaving::BitVector *bitvector = new bitweaving::BitVector(*table);
    column1 = table->GetColumn(std::to_string(0));
    column1->Scan_between(lowerBoundQuery[0],upperBoundQuery[0],kAnd, bitvector);
    
    for(uint32_t dim = 1; dim<STORE->NUM_DIM;dim++){
        
        column1 = table->GetColumn(std::to_string(dim));
        if(lowerBoundQuery[dim]== upperBoundQuery[dim]){
            column1->Scan(lowerBoundQuery[dim], kEqual,kAnd, bitvector);
        }else{
#ifdef BW_TWO_PASS
            
                column1->Scan(lowerBoundQuery[dim],kGreaterEqual, kAnd, bitvector);
                column1->Scan(upperBoundQuery[dim],kLessEqual, kAnd, bitvector);
#else
                column1->Scan_between(lowerBoundQuery[dim],upperBoundQuery[dim],kAnd, bitvector);
#endif
        }
        
    }
    
    bitvector->toTids(resultTIDs,STORE->NUM_POINTS);
    
    // Create an iterator to get matching values
    /*bitweaving::Iterator *iter = new bitweaving::Iterator(*bitvector);
    
    while (iter->Next()) {
        // Get the next matching value
        resultTIDs->push_back((int32_t)iter->GetPosition());
    }
    
    delete iter;//*/
    delete bitvector;
    
    return resultTIDs;
}

vector<uint32_t>* BitWeaving::partialMatch(uint32_t* lowerBoundQuery, uint32_t* upperBoundQuery, bool* columnsForSelect){
    vector<uint32_t>* resultTIDs = new vector<uint32_t>();;
    
    
    bitweaving::Column *column1;
    bitweaving::BitVector *bitvector = new bitweaving::BitVector(*table);
    
    uint32_t dim = 0;
    while (!columnsForSelect[dim]) {
        dim++;
    }
    
    column1 = table->GetColumn(std::to_string(dim));
    column1->Scan_between(lowerBoundQuery[dim],upperBoundQuery[dim], bitvector);
    
    for(; dim<STORE->NUM_DIM;dim++){
        if(columnsForSelect[dim]){
            column1 = table->GetColumn(std::to_string(dim));
            if(lowerBoundQuery[dim]== upperBoundQuery[dim]){
                column1->Scan(lowerBoundQuery[dim], kEqual,kAnd, bitvector);
            }else{
#ifdef BW_TWO_PASS
            
                column1->Scan(lowerBoundQuery[dim],kGreaterEqual, kAnd, bitvector);
                column1->Scan(upperBoundQuery[dim],kLessEqual, kAnd, bitvector);
#else
                column1->Scan_between(lowerBoundQuery[dim],upperBoundQuery[dim],kAnd, bitvector);
#endif
            }
        }
        
    }
    
    bitvector->toTids(resultTIDs,STORE->NUM_POINTS);
    // Create an iterator to get matching values
    /*
    bitweaving::Iterator *iter = new bitweaving::Iterator(*bitvector);
    
    while (iter->Next()) {
        // Get the next matching value
        resultTIDs->push_back((int32_t)iter->GetPosition());
    }
    
    delete iter;//*/
    delete bitvector;
    
    return resultTIDs;
}
