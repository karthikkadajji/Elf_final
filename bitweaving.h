//
//  bitweaving.h
//  dwarf
//
//  Created by David Broneske on 13.01.15.
//  Copyright (c) 2015 David Broneske. All rights reserved.
//

#ifndef __dwarf__bitweaving__
#define __dwarf__bitweaving__

#include <stdio.h>

#include "bitweaving/column.h"
#include "bitweaving/status.h"
#include "bitweaving/table.h"
#include "bitweaving/types.h"

#include "Store.h"
#include "Index.h"
#include "Util.h"

using namespace bitweaving;
class BitWeaving : Index
{
public:
    Table* table;
    ColumnType ctype;
    int* code_sizes;
    
    BitWeaving(Store* s, ColumnType columnType, int* sizes) : Index(s), ctype(columnType), code_sizes(sizes){
 
    }
    void buildIndex();
    
    uint32_t exactMatch(uint32_t* query);
    
    vector<uint32_t>* partialMatch(uint32_t* lowerBoundQuery, uint32_t* upperBoundQuery, bool* columnsForSelect);
    vector<uint32_t>* windowQuery(uint32_t* lowerBoundQuery, uint32_t* upperBoundQuery);
    ~BitWeaving(){
        delete table;
    }
};


#endif /* defined(__dwarf__bitweaving__) */
