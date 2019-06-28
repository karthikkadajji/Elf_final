//
//  colseqscan.h
//  dwarf
//
//  Created by David Broneske on 08.07.15.
//  Copyright (c) 2015 David Broneske. All rights reserved.
//

#ifndef sortedProj_h
#define sortedProj_h

#include "SortingEngine/sortingEngine.h"
#include <string.h>
#include <vector>
#include <algorithm>

#include "Index.h"

bool tupleComp(vector<uint32_t>* left, vector<uint32_t>* right) {
  //  for (int i = 0; i < left->size() - 1; i++) {
        return (*left)[0] < (*right)[0];
   // }
   // return false;
}

class SortedProjection : Index {
    
private:


    vector<pair<vector<uint32_t>* , vector<uint32_t*>* > >* cols;
    vector<uint32_t>* lastDims;
    vector<uint32_t>* firstDims;
    uint32_t* max_dims;



public:

    SortedProjection(Store* s, vector<uint32_t>& beginDims, vector<uint32_t>& endDims, uint32_t* maxDims) : Index(s), max_dims(maxDims) {
        firstDims = new vector<uint32_t>(beginDims.begin(), beginDims.end());
        lastDims = new vector<uint32_t>(endDims.begin(), endDims.end());
        cols = new vector<pair<vector<uint32_t>* , vector<uint32_t*>* > >();
        vector<uint32_t*>* row;
        vector<uint32_t>* col;
        for (int i = 0; i < lastDims->size(); i++) {
            col=new vector<uint32_t>(STORE->NUM_POINTS);
            row=new vector<uint32_t*>(STORE->NUM_POINTS);
            for(int j=0; j < STORE->NUM_POINTS; j++){
                row->at(j)=(uint32_t*)malloc((*lastDims)[i]-(*firstDims)[i]+1);
            }
            cols->push_back(pair<vector<uint32_t>* , vector<uint32_t*>* >(col, row)); 
        }
    };

    ~SortedProjection() {

        delete lastDims;
        delete firstDims;

        for (int i = 0; i < cols->size(); i++) {
            delete cols->at(i).first;
            
            for(int j=0; j < STORE->NUM_POINTS; j++){
                delete cols->at(i).second->at(j);
            } 
            delete cols->at(i).second;
        }
        delete cols;
    }

    void buildIndex() {
        uint32_t* temp;
        Array<uint32_t> data_array;
        data_array.ary=(TID<uint32_t>*)malloc(STORE->NUM_POINTS*sizeof(TID<uint32_t>));
        data_array.size=STORE->NUM_POINTS;
        for (int numSPs = 0; numSPs < firstDims->size(); numSPs++) {
            
            temp=(uint32_t*) malloc(((*lastDims)[numSPs]-(*firstDims)[numSPs] + 2)*STORE->NUM_POINTS*sizeof(uint32_t));
            for (uint64_t i = 0; i < STORE->NUM_POINTS; i++) {
                memcpy(&(temp[((*lastDims)[numSPs]-(*firstDims)[numSPs] + 2)*i]), &(STORE->getPoint(i)[(*firstDims)[numSPs]]), ((*lastDims)[numSPs]-(*firstDims)[numSPs]+1) * sizeof (uint32_t));
                temp[(((*lastDims)[numSPs]-(*firstDims)[numSPs] + 2)*i)
                        +(*lastDims)[numSPs]-(*firstDims)[numSPs]+1] = i;
                data_array.ary[i].value=&(temp[((*lastDims)[numSPs]-(*firstDims)[numSPs] + 2)*i]);
            }
            sort(&data_array, 0, max_dims,(*lastDims)[numSPs]-(*firstDims)[numSPs] + 2);

            for (uint32_t dim = 0; dim < ((*lastDims)[numSPs]-(*firstDims)[numSPs])+2; dim++) {
                for (uint32_t point = 0; point < STORE->NUM_POINTS; point++) {
                    cols->at(numSPs).first->at(point) = data_array.ary[point].value[0];
                    memcpy(&(cols->at(numSPs).second->at(point)[0]), &(data_array.ary[point].value[1]), ((*lastDims)[numSPs]-(*firstDims)[numSPs]+1) * sizeof (uint32_t));
               
                }
            }
                free(temp);
        }
    }

    uint32_t exactMatch(uint32_t* query) {

        std::vector<uint32_t>::iterator it;
        int i;
        uint32_t tid;
        it = lower_bound(((*cols)[0]).first->begin(), ((*cols)[0]).first->end(), query[0]);
        if (it != ((*cols)[0]).first->end()) {
            while ((*it) == query[0] && it != ((*cols)[0]).first->end()) {
                tid = it - ((*cols)[0]).first->begin();
                for (i = 1; i < (*lastDims)[0]; i++) {
                    if (((*cols)[0]).second->at(tid)[i-1] != query[i])
                        break;
                }
                if (i >= (*lastDims)[0])
                    return ((*cols)[0]).second->at(tid)[i];
                it++;
            }
        }
        return NOT_FOUND;
    }

    vector<uint32_t>* partialMatch(uint32_t* lowerBoundQuery, uint32_t* upperBoundQuery, bool* columnsForSelect) {
        vector<uint32_t>* resultTIDs = new vector<uint32_t>();
        uint32_t tid;
        uint32_t end;
        uint32_t dim;
        for (dim = 0; dim < STORE->NUM_DIM; dim++) {
            if (columnsForSelect[dim]) {
                break;
            }
        }
        int numSPs;
        for (numSPs = 0; numSPs < cols->size(); numSPs++) {
            if ((*firstDims)[numSPs] == dim)
                break;
        }



        if (numSPs >= cols->size())
            cout << "No suitable Projection found" << endl;

        pair<vector<uint32_t>* , vector<uint32_t*>* >  sortedProj = (*cols)[numSPs];
        std::vector<uint32_t>::iterator itlow;
        std::vector<uint32_t>::iterator itup;
        itlow = lower_bound(sortedProj.first->begin(), sortedProj.first->end(), lowerBoundQuery[dim]);
        itup = upper_bound(sortedProj.first->begin(), sortedProj.first->end(), upperBoundQuery[dim]);
        tid = itlow - sortedProj.first->begin();
        end = itup - sortedProj.first->begin();
        for (;tid < end; tid++) {
            partialMatchTuple(dim+1, tid, sortedProj.second, lastDims->at(numSPs),firstDims->at(numSPs), resultTIDs, lowerBoundQuery, upperBoundQuery, columnsForSelect);
        }



        return resultTIDs;
    }


    inline void partialMatchTuple(uint32_t dim,uint32_t tid, vector<uint32_t*>* sortedProj, uint32_t lastDims,uint32_t firstDims, vector<uint32_t>* RESULTS, uint32_t* lowerBoundQuery, uint32_t* upperBoundQuery, bool* columnsForSelect) {
        uint32_t value;
        uint32_t i=0;
        for (; dim+i <= lastDims; i++) {
            if (columnsForSelect[dim+i]) {
                value = sortedProj->at(tid)[i];
                if (!isIn(lowerBoundQuery[dim+i], upperBoundQuery[dim+i], value))
                    return;
            }
        }
        RESULTS->push_back(sortedProj->at(tid)[lastDims+1]);
    }

    vector<uint32_t>* windowQuery(uint32_t* lowerBoundQuery, uint32_t* upperBoundQuery) {
        vector<uint32_t>* resultTIDs = new vector<uint32_t>();

        cout << "currently not implemented" << endl;


        return resultTIDs;
    }
};

#endif
