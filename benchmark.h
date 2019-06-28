//
//  benchmark.h
//  dwarf
//
//  Created by David Broneske on 02.03.15.
//  Copyright (c) 2015 David Broneske. All rights reserved.
//

#ifndef dwarf_benchmark_h
#define dwarf_benchmark_h




uint64_t getTimestamp(){
    boost::chrono::high_resolution_clock::time_point tp = boost::chrono::high_resolution_clock::now();
    boost::chrono::nanoseconds dur = tp.time_since_epoch();
    
    return (uint64_t) dur.count();
    
}

float evalEM(Index* runMe){
    uint32_t result;
    uint32_t anzIter=10;
    
    uint64_t start;
    runMe->buildIndex();
    
    start = getTimestamp();
    for(uint32_t tid=0;tid<anzIter;tid++){
        result = runMe->exactMatch(runMe->STORE->getPoint(runMe->STORE->NUM_POINTS-1));
    }
    
    return ((float)(getTimestamp()-start))/1000;
}

float evalRangeQuery(Index* runMe, RangeQueries* ranges){
    vector<uint32_t>* resultsTIDs;
    
    uint64_t start = getTimestamp();
    for(uint32_t query=0;query<ranges->NUM_QUERIES;query++){
        resultsTIDs = runMe->windowQuery(ranges->lowerBound[query], ranges->upperBound[query]);
        delete resultsTIDs;
    }
    return ((float)(getTimestamp()-start))/1000;
}


#endif
