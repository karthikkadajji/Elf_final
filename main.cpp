

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "Store.h"
#include "seqscan.h"
#include "colseqscan.h"
#ifndef NO_SIMD_OPERATIONS
#include "simdseqscan.h"
#endif
#include <boost/chrono.hpp>
#include "Index.h"
#include "time.h"
#include <vector>
#include <algorithm>
#include "Imprint.h"
#include "MyKDTreeSmall.h"
#include "ImprintWithoutCompression.h"
#ifndef NO_SIMD_OPERATIONS
#include "ImprintSIMDWithoutCompression.h"
#include "ImprintSIMD64WithoutCompression.h"
#endif
#include "tpc_h.h"

#include "Elf_final64.h"
#include "Elf_final64_nodelevel_enqueue.h"
#include "Elf_final64_subtree.h"

#include "sortedProjections.h"


#include "limits.h"
#include "benchmark.h"

#if defined  BITWEAVINGV || BITWEAVINGH
#include "bitweaving.h"
#endif

#include <string.h>
#include <map>


#include "SortingEngine/sortingEngine.h"
#include<algorithm>
#include<set>



using namespace std;
vector<uint32_t >result_ELF;
vector<uint32_t> result;
vector<uint32_t> result_difference;
vector<uint32_t > result_ELF_subtree;
vector<uint32_t >result_ELF_nodelevel_noenqueue;
vector<uint32_t >result_ELF_nodelevel_enqueue;

struct mygreater {

    bool operator()(uint32_t * const &a, uint32_t * const &b) const {
        return a[0] < b[0];
    }
} mygreater;

static int byteArrayToInt(char* b) {
    return (b[3] & 0xFF) | (b[2] & 0xFF) << 8 | (b[1] & 0xFF) << 16
            | (b[0] & 0xFF) << 24;
}

void sort_r(Array<uint32_t>& data_array, int dim, int numDim, uint32_t* max) {
    if (dim < numDim) {
        sort(&data_array, dim, max, numDim);

        uint32_t cur = data_array.ary[0].value[dim];
        uint32_t begin = 0;
        Array<uint32_t> temp;
        int i;
        for (i = 0; i < data_array.size; i++) {
            if (cur != data_array.ary[i].value[dim]) {
                temp.ary = &(data_array.ary[begin]);
                temp.size = i - begin;
                sort_r(temp, dim + 1, numDim, max);
                begin = i;
                cur = data_array.ary[i].value[dim];
            }
        }

        temp.ary = &(data_array.ary[begin]);
        temp.size = i - begin;
        sort_r(temp, dim + 1, numDim, max);
        begin = i;
    }
}

uint32_t** readTBL(uint32_t* numPoints, uint32_t* numDimRef, uint32_t* dimMaxValue, char* filePath, uint32_t** maxDims) {
    ifstream myfile;
    myfile.open(filePath, ios::in | ios::binary);
    uint32_t** store;
    vector<uint32_t>* min=new vector<uint32_t>;
    vector<uint32_t>* max=new vector<uint32_t>;

    if (myfile.is_open()) {
        cout << "Start reading: " << filePath << endl;
        //std::string line;

        char* bytes = (char*) malloc(4);
        myfile.read(bytes, 4);
        int numRows = byteArrayToInt(bytes);
        *numPoints = numRows;
        myfile.read(bytes, 4);
        int numDim = byteArrayToInt(bytes);
        *numDimRef = numDim;
        //to be adapted
        *dimMaxValue = INT_MAX;


        for (int i = 0; i < numDim; i++) {
            min->push_back(INT_MAX);
            max->push_back(0);
        }

        store = new uint32_t*[numRows];
        for (int row = 0; row < numRows; row++) {
            store[row] = new uint32_t[numDim];

            for (int dim = 0; dim < numDim; dim++) {

                //                char temp;
                myfile.read(bytes, 4);
                store[row][dim] = (uint32_t) byteArrayToInt(bytes);

                    if ((*min)[dim] > store[row][dim])
                        (*min)[dim] = store[row][dim];
                    if ((*max)[dim] < store[row][dim])
                        (*max)[dim] = store[row][dim];

               /* if (store[row][0] == 0 && dim==numDim-1) {


                for (int dim2 = 0; dim2 < numDim + 1; dim2++) {
                    cout << store[row][dim2] << ", ";
                }
                cout << endl;
               }*/
            }
        }
        cout << "Stop reading!" << endl;
        myfile.close();
        free(bytes);

         cout << "Ranges: ";
        for (int dim = 0; dim < numDim; dim++) {
            cout << "[ " << (*min)[dim] << ", " << (*max)[dim] << "], ";

        }
        cout << endl;
        (*maxDims)=&((*max)[0]);


    } else {
        cout << "Could not open file" << endl;
        exit(-1);
    }

    //vector<uint32_t*>* temp = new vector<uint32_t*>(store, store + *numPoints);
    //std::sort(temp->begin(), temp->end(), mygreater);
    return store;
}




uint32_t ** getData(uint32_t size, uint32_t dim, uint32_t dimMaxValue, uint32_t seed) {
    uint32_t ** data;

    data = new uint32_t*[size];
    for (uint32_t i = 0; i < size; i++) {
        data[i] = new uint32_t[dim];
    }

    cout << "getData(): Size of dataset " << endl;
    srand(seed);

    for (uint32_t point = 0; point < size; point++) {
        for (uint32_t value = 0; value < dim; value++) {
            data[point][value] = (rand() % dimMaxValue);
        }
    }
    vector<uint32_t*>* temp = new vector<uint32_t*>(data, data + size);
    //std::sort(temp->begin(),temp->end(),mygreater);
    return &(temp->at(0));
}

void buildIndex(Index* runMe) {

    uint64_t start = getTimestamp();
    runMe->buildIndex();
    cout << "Time build in ms: " << ((float) (getTimestamp() - start)) / 1000000 << endl;
}

void UTrunExactMatch(Index* runMe) {
    uint32_t result;


    uint64_t start = getTimestamp();
    for (uint32_t numQuery = 0; numQuery < runMe->STORE->NUM_POINTS; numQuery++) {
        //        cout << "Query: ";
        //        for (int i = 0; i < runMe->STORE->NUM_DIM; i++) {
        //            cout << runMe->STORE->getPoint(tid)[i] <<", ";
        //        }
        //        cout << endl;

        result = runMe->exactMatch(runMe->STORE->getPoint(numQuery));
        //if (numQuery % 1000 ==0)
        if (numQuery != result)
            cout << numQuery << ":" << result << " " << flush;
    }
    cout << endl << "Time exact match in ms: " << ((float) (getTimestamp() - start)) / 1000000 << endl;
}

void runExactMatch(Index* runMe, EMQueries* emqueries) {
    uint32_t result;
    uint64_t start;
    std::cout << "Time exact match in ms: ";
    //  for(uint32_t reps = 0; reps < NUM_Reps; reps++){
    start = getTimestamp();
    for (uint32_t numQuery = 0; numQuery < emqueries->NUM_QUERIES; numQuery++) {
        result = runMe->exactMatch(emqueries->pointsQueries[numQuery]);
        //if (numQuery % 1000 == 0)
        //    cout << emqueries->pointsQueries[numQuery][runMe->STORE->NUM_DIM] << ":" << result << " " << flush;
    }
    std::cout << ((float) (getTimestamp() - start)) / 1000000 << ";";
    // }
    cout << endl;

}

void outVector(uint32_t query, vector<uint32_t>* resultsTIDs) {
    cout << query << ":[";

    for (uint32_t result = 0; result < resultsTIDs->size(); result++) {
        cout << (*resultsTIDs)[result] << ", ";
    }
    cout << "]" << endl;

}

void outQuery(uint32_t queryNo, RangeQueries* queries) {
    cout << queryNo << ":[";

    for (uint32_t dim = 0; dim < queries->NUM_DIM; dim++) {
        cout << "(" << (queries->lowerBound)[queryNo][dim] << ", " << (queries->upperBound)[queryNo][dim] << ")" << ", ";
    }
    cout << "]" << endl;

}


/**
 * Does not call buildIndex().
 */
void runRangeQuery(Index* runMe, RangeQueries* ranges) {
    vector<uint32_t>* resultsTIDs;

    uint64_t start = getTimestamp();
    for (uint32_t query = 0; query < ranges->NUM_QUERIES; query++) {

        //            outQuery(query, ranges);

        resultsTIDs = runMe->windowQuery(ranges->lowerBound[query], ranges->upperBound[query]);
        //if(query%10000==0)
        //   outVector(query, resultsTIDs);
        // cout << resultsTIDs.size()*100/runMe->STORE->NUM_POINTS<<endl;
        delete resultsTIDs;

    }
    std::cout << "Time range query in ms: " << ((float) (getTimestamp() - start)) / 1000000 << endl;
}

void runPartialMatchQuery(Index* runMe, PartialMatchQueries* ranges) {
    vector<uint32_t>* resultTIDs;

    uint64_t start = getTimestamp();
    cout << "Size: ";
    for (uint32_t query = 0; query < ranges->NUM_QUERIES; query++) {
        //            outQuery(query, ranges);

        resultTIDs = runMe->partialMatch(ranges->lowerBound[query], ranges->upperBound[query], ranges->columnsForSelect);
        cout << resultTIDs->size() << ", ";
        /*            if(query%500==0){
                       sort(resultTIDs->begin(), resultTIDs->end());
                       cout << "Size: " << endl;
                       outVector(query, resultTIDs);
                    }
         */ // cout << resultsTIDs.size()*100/runMe->STORE->NUM_POINTS<<endl;
        delete resultTIDs;
    }

#ifdef UnitTest
    cout << endl;
#endif
    std::cout << "Time partial match query in ms: " << ((float) (getTimestamp() - start)) / 1000000 << endl;
}

vector <uint32_t > runQ1Query(Index* runMe, PartialMatchQueries* ranges) {
    vector<uint32_t>* resultTIDs;
    result.clear();
    uint64_t start;
#ifdef UnitTest
    cout << "Size: ";
#endif
    std::cout << "Time Q1 in ms: ";
    for (uint32_t reps = 0; reps < NUMREPS; reps++) {
        start = getTimestamp();
        for (uint32_t query = 0; query < ranges->NUM_QUERIES; query++) {
            //            outQuery(query, ranges);

            resultTIDs = runMe->partialMatch(ranges->lowerBound[query], ranges->upperBound[query], ranges->columnsForSelect);
#ifdef UnitTest

            cout << resultTIDs->size() << ", ";
            result.insert(result.end(), resultTIDs->begin(), resultTIDs->end());
#endif
            /*            if(query%500==0){
             sort(resultTIDs->begin(), resultTIDs->end());
             cout << "Size: " << endl;
             outVector(query, resultTIDs);
             }
             */ // cout << resultsTIDs.size()*100/runMe->STORE->NUM_POINTS<<endl;
            delete resultTIDs;
        }
        std::cout << ((float) (getTimestamp() - start)) / 1000000 << ";";
    }
    cout << endl;
    return result;
}

vector<uint32_t > runQ6Query(Index* runMe, PartialMatchQueries* ranges) {
    vector<uint32_t>* resultTIDs;
    result.clear();
    uint64_t start;
#ifdef UnitTest
    cout << "Size: ";
#endif
    std::cout << "Time Q6 in ms: ";
    for (uint32_t reps = 0; reps < NUMREPS; reps++) {
        start = getTimestamp();
        for (uint32_t query = 0; query < ranges->NUM_QUERIES; query++) {
            //            outQuery(query, ranges);

            resultTIDs = runMe->partialMatch(ranges->lowerBound[query], ranges->upperBound[query], ranges->columnsForSelect);
#ifdef UnitTest
            cout << resultTIDs->size() << ", ";
            result.insert(result.end(), resultTIDs->begin(), resultTIDs->end());
#endif

            delete resultTIDs;
        }
        std::cout << ((float) (getTimestamp() - start)) / 1000000 << ";";
    }
    cout << endl;
    return result;
}

vector<uint32_t> runQ10Query(Index* runMe, PartialMatchQueries* ranges) {
    vector<uint32_t>* resultTIDs;
    result.clear();
    uint64_t start;
#ifdef UnitTest
    cout << "Size: ";
#endif
    std::cout << "Time Q10 in ms: ";
    for (uint32_t reps = 0; reps < NUMREPS; reps++) {
        start = getTimestamp();
        for (uint32_t query = 0; query < ranges->NUM_QUERIES; query++) {
            //            outQuery(query, ranges);

            resultTIDs = runMe->partialMatch(ranges->lowerBound[query], ranges->upperBound[query], ranges->columnsForSelect);
#ifdef UnitTest
            cout << resultTIDs->size() << ", ";
            result.insert(result.end(), resultTIDs->begin(), resultTIDs->end());
#endif

            delete resultTIDs;
        }
        std::cout << ((float) (getTimestamp() - start)) / 1000000 << ";";
    }
    cout << endl;
    return result;
}

vector<uint32_t >runQ14Query(Index* runMe, PartialMatchQueries* ranges) {
    vector<uint32_t>* resultTIDs;
    result.clear();
    uint64_t start;
#ifdef UnitTest
    cout << "Size: ";
#endif
    std::cout << "Time Q14 in ms: ";
    for (uint32_t reps = 0; reps < NUMREPS; reps++) {
        start = getTimestamp();
        for (uint32_t query = 0; query < ranges->NUM_QUERIES; query++) {
            //            outQuery(query, ranges);

            resultTIDs = runMe->partialMatch(ranges->lowerBound[query], ranges->upperBound[query], ranges->columnsForSelect);
#ifdef UnitTest
            cout << resultTIDs->size() << ", ";
            result.insert(result.end(), resultTIDs->begin(), resultTIDs->end());
#endif

            delete resultTIDs;
        }
        std::cout << ((float) (getTimestamp() - start)) / 1000000 << ";";
    }
    cout << endl;
    return result;
}

vector<uint32_t > runQ17Query(Index* runMe, PartialMatchQueries* ranges) {
    vector<uint32_t>* resultTIDs;
    result.clear();

    uint64_t start;
#ifdef UnitTest
    cout << "Size: ";
#endif
    std::cout << "Time Q17 in ms: ";
    for (uint32_t reps = 0; reps < NUMREPS; reps++) {
        start = getTimestamp();
        for (uint32_t query = 0; query < ranges->NUM_QUERIES; query++) {
            //            outQuery(query, ranges);

            resultTIDs = runMe->partialMatch(ranges->lowerBound[query], ranges->upperBound[query], ranges->columnsForSelect);
#ifdef UnitTest
            cout << resultTIDs->size() << ", ";
            result.insert(result.end(), resultTIDs->begin(), resultTIDs->end());

#endif
            delete resultTIDs;
        }
        std::cout << ((float) (getTimestamp() - start)) / 1000000 << ";";
    }
    cout << endl;
    return result;
}

vector<uint32_t > runQ19Query(Index* runMe, PartialMatchQueries* ranges1, PartialMatchQueries* ranges2, PartialMatchQueries* ranges3) {
    vector<uint32_t>* resultTIDs1;
    vector<uint32_t>* resultTIDs2;
    vector<uint32_t>* resultTIDs3;
    result.clear();

    uint64_t start;
#ifdef UnitTest
    cout << "Size: ";
#endif

    std::cout << "Time Q19 in ms: ";
    for (uint32_t reps = 0; reps < NUMREPS; reps++) {
        start = getTimestamp();
        for (uint32_t query = 0; query < ranges1->NUM_QUERIES; query++) {\
            resultTIDs1 = runMe->partialMatch(ranges1->lowerBound[query], ranges1->upperBound[query], ranges1->columnsForSelect);
            resultTIDs2 = runMe->partialMatch(ranges2->lowerBound[query], ranges2->upperBound[query], ranges2->columnsForSelect);
            resultTIDs3 = runMe->partialMatch(ranges3->lowerBound[query], ranges3->upperBound[query], ranges3->columnsForSelect);

#ifdef UnitTest
            cout << resultTIDs1->size() << ", ";
            cout << resultTIDs2->size() << ", ";
            cout << resultTIDs3->size() << ", ";
            result.insert(result.end(), resultTIDs1->begin(), resultTIDs1->end());
            result.insert(result.end(), resultTIDs2->begin(), resultTIDs2->end());
            result.insert(result.end(), resultTIDs3->begin(), resultTIDs3->end());
#endif

            delete resultTIDs1;
            delete resultTIDs2;
            delete resultTIDs3;


        }
        std::cout << ((float) (getTimestamp() - start)) / 1000000 << ";";
    }

#ifdef UnitTest
    cout << endl;
#endif
    return result;
}


void compare_results(string query, string table_name, vector<uint32_t> result_vector){
    if (table_name == "Part")
    {
        std::sort(result_ELF.begin(),result_ELF.end());
        std::sort(result_vector.begin(), result_vector.end());
        set_difference(result_ELF.begin(), result_ELF.end(), result_vector.begin(), result_vector.end(),
                       std::inserter(result_difference, result_difference.end()));

        int size_unmatched = result_difference.size();
        if (size_unmatched != 0)
        {
            cout << "Result didn't match for" << query << endl << endl;
        }

        result_difference.clear();
        result_vector.clear();
    }

    else if (table_name == "Line")
    {
        std::sort(result_ELF.begin(),result_ELF.end());
        std::sort(result_vector.begin(), result_vector.end());
        set_difference(result_ELF.begin(), result_ELF.end(), result_vector.begin(), result_vector.end(),
                       std::inserter(result_difference, result_difference.end()));

        int size_unmatched = result_difference.size();
        if (size_unmatched != 0)
        {
            cout << "RESULT DIDN'T MATCH FOR" << query << endl << endl;
            exit(-1);
        }

        result_difference.clear();
        result_vector.clear();
    }

}

void clear_vectors()
{
    result_ELF.clear();
//    result_COL_SEQ.clear();
    result_difference.clear();
    result_ELF_subtree.clear();
    result_ELF_nodelevel_noenqueue.clear();
    result_ELF_nodelevel_enqueue.clear();
    result.clear();

}

int main(int argc, char* argv[]) {
    uint32_t numPoints;
    uint32_t numDim;
    uint32_t dimMaxValue;
    uint32_t** data;
    Store* s;
    uint32_t* maxDims;
    RangeQueries* ranges;
    EMQueries* exacts;
    PartialMatchQueries* partialsTPCH1;
    PartialMatchQueries* partialsTPCH2;
    PartialMatchQueries* partialsTPCH3;
    PartialMatchQueries* partialsTPCH4;
#ifdef LineItem
    PartialMatchQueries* partialsTPCH5;
    PartialMatchQueries* partialsTPCH6;
    PartialMatchQueries* partialsTPCH7;
#endif


    if (argc > 1) {


        data = readTBL(&numPoints, &numDim, &dimMaxValue, argv[1], &maxDims);

#ifdef REDUCED_DIM
        numDim = 7;
#endif
        s = new Store(numDim, numPoints, 0, dimMaxValue);
        s->bulkInsertWithoutCheck(data);
        ranges = new RangeQueries(NUM_Range, s);
        exacts = new EMQueries(NUM_Exact, s);

        bool *dimsForSelection1 = new bool[ranges->NUM_DIM];
        bool *dimsForSelection2 = new bool[ranges->NUM_DIM];
#ifdef LineItem
        bool *dimsForSelection3 = new bool[ranges->NUM_DIM];
        bool *dimsForSelection4 = new bool[ranges->NUM_DIM];
#endif

#ifdef LineItem
#ifdef REDUCED_DIM
        if(numDim!=7)
#else
        if (numDim != 15)
#endif
        {
            cout << "wrong Table loaded!" << endl;
            exit(-1);
        }
        partialsTPCH1 = new PartialMatchQueries(NUM_Partials, s, dimsForSelection1);
        partialsTPCH2 = new PartialMatchQueries(NUM_Partials, s, dimsForSelection2);
        updateQ1(partialsTPCH1);
        updateQ6(partialsTPCH2);
        partialsTPCH3 = new PartialMatchQueries(NUM_Partials, s, dimsForSelection3);
        updateQ10(partialsTPCH3);
        partialsTPCH4 = new PartialMatchQueries(NUM_Partials, s, dimsForSelection1);
        updateQ14(partialsTPCH4);
        partialsTPCH5 = new PartialMatchQueries(NUM_Partials, s, dimsForSelection4);
        partialsTPCH6 = new PartialMatchQueries(NUM_Partials, s, dimsForSelection4);
        partialsTPCH7 = new PartialMatchQueries(NUM_Partials, s, dimsForSelection4);
        updateLQ19_1(partialsTPCH5);
        updateLQ19_2(partialsTPCH6);
        updateLQ19_3(partialsTPCH7);
#endif
#ifdef Part
#ifdef REDUCED_DIM
        if(numDim!=7)
#else
        if(numDim!=9)
#endif
        {
             cout << "wrong Table loaded!" << endl;
             exit(-1);
        }
        partialsTPCH1 = new PartialMatchQueries(NUM_Partials, s, dimsForSelection1);
        partialsTPCH2 = new PartialMatchQueries(NUM_Partials, s, dimsForSelection2);
        partialsTPCH3 = new PartialMatchQueries(NUM_Partials, s, dimsForSelection2);
        partialsTPCH4 = new PartialMatchQueries(NUM_Partials, s, dimsForSelection2);
        updateQ17(partialsTPCH1);
        updateQ19_1(partialsTPCH2);
        updateQ19_2(partialsTPCH3);
        updateQ19_3(partialsTPCH4);
        cout << endl << endl << endl << endl << endl << endl << endl << endl << endl << endl;
#endif
    }else{
        cout << "Specify a .tbl file as input." << endl;
        exit(-1);
    }
    cout << endl << "Number of Points: " << numPoints << ", Number of Dimensions: " << numDim << ", MaxValue: " << dimMaxValue << endl << endl;


    for (int queries = 0; queries < NUMREPS; queries++) {


//#ifdef SEQ
//        cout << endl << "SEQ" << endl;
//        Index* scan = (Index*) new SeqScan(s);
//        buildIndex(scan);
//        runExactMatch(scan, exacts);
//#ifdef LineItem
//        runQ1Query(scan, partialsTPCH1);
//        runQ6Query(scan, partialsTPCH2);
//        runQ10Query(scan, partialsTPCH3);
//        runQ14Query(scan, partialsTPCH4);
//        runQ19Query(scan, partialsTPCH5, partialsTPCH6, partialsTPCH7);
//#endif
//#ifdef Part
//        runQ17Query(scan, partialsTPCH1);
//        runQ19Query(scan, partialsTPCH2, partialsTPCH3, partialsTPCH4);
//        cout << endl << endl << endl << endl << endl << endl << endl << endl << endl << endl;
//#endif
//
//#endif
//
/*#ifdef COLSEQ
        cout << endl << "COL_SEQ" << endl;
        Index* colscan = (Index*) new Colseqscan(s);
        buildIndex(colscan);
        runExactMatch(colscan, exacts);
#ifdef LineItem
        runQ1Query(colscan, partialsTPCH1);
        runQ6Query(colscan, partialsTPCH2);
        runQ10Query(colscan, partialsTPCH3);
        runQ14Query(colscan, partialsTPCH4);
        runQ19Query(colscan, partialsTPCH5, partialsTPCH6, partialsTPCH7);
#endif
#ifdef Part
        runQ17Query(colscan, partialsTPCH1);
        runQ19Query(colscan, partialsTPCH2, partialsTPCH3, partialsTPCH4);
        cout << endl << endl;
#endif
        delete (Colseqscan*) colscan;
#endif*/


#ifdef ElfFinalBigPtr
        cout << endl << "ELF_FINAL64" << endl;
        Index* elfFinal64 = (Index*) new Elf_final64(s, maxDims);
        buildIndex(elfFinal64);
        Index* elfFinal64_subtree = (Index*) new Elf_final64_subtree(s, maxDims);
        buildIndex(elfFinal64_subtree);
        Index* elfFinal64_nodelevel_enqueue = (Index*) new Elf_final64_nodelevel_enqueue(s, maxDims);
        buildIndex(elfFinal64_nodelevel_enqueue);
        unsigned int thread_limit = std::thread::hardware_concurrency();

#ifdef SELTEST
        calculateSelectivities(elfFinal64);
        calculateSelectivities(elfFinal64_subtree);
#else
#ifdef UnitTest
        UTrunExactMatch(elfFinal64);
#else
        runExactMatch(elfFinal64, exacts);
        runExactMatch(elfFinal64_subtree, exacts);
#endif
#endif

#ifdef LineItem
        for (int i =1 ; i <= thread_limit ; i = i + 1)
            {


                cout << "Running for threads " << i << endl;
                elfFinal64_subtree->setNumThreads(i);
                elfFinal64_nodelevel_enqueue->setNumThreads(i);
                cout << endl;


                cout << "Sequential Elf" << endl;
                result_ELF = runQ1Query(elfFinal64, partialsTPCH1);
                cout << "Sub-tree" << endl;
                result_ELF_subtree = runQ1Query(elfFinal64_subtree, partialsTPCH1);
                compare_results("runQ1Query", "Line", result_ELF_subtree);
                cout << "Node level" << endl;
                result_ELF_nodelevel_enqueue = runQ1Query(elfFinal64_nodelevel_enqueue, partialsTPCH1);
                compare_results("runQ1Query", "Line", result_ELF_nodelevel_enqueue);
                clear_vectors();
                cout << endl;

                cout << "Sequential Elf" << endl;
                result_ELF = runQ6Query(elfFinal64, partialsTPCH2);
                cout << "Sub-tree" << endl;
                result_ELF_subtree = runQ6Query(elfFinal64_subtree, partialsTPCH2);
                compare_results("runQ6Query", "Line", result_ELF_subtree);
                cout << "Node level" << endl;
                result_ELF_nodelevel_enqueue = runQ6Query(elfFinal64_nodelevel_enqueue, partialsTPCH2);
                compare_results("runQ6Query", "Line", result_ELF_nodelevel_enqueue);
                clear_vectors();
                cout << endl;


                cout << "Sequential Elf" << endl;
                result_ELF = runQ10Query(elfFinal64, partialsTPCH3);
                cout << "Sub-tree" << endl;
                result_ELF_subtree = runQ10Query(elfFinal64_subtree, partialsTPCH3);
                compare_results("runQ10Query", "Line", result_ELF_subtree);
                cout << "Node level" << endl;
                result_ELF_nodelevel_enqueue = runQ10Query(elfFinal64_nodelevel_enqueue, partialsTPCH3);
                compare_results("runQ10Query", "Line", result_ELF_nodelevel_enqueue);
                clear_vectors();
                cout << endl;

                cout << "Sequential Elf" << endl;
                result_ELF = runQ14Query(elfFinal64, partialsTPCH4);
                cout << "Sub-tree" << endl;
                result_ELF_subtree = runQ14Query(elfFinal64_subtree, partialsTPCH4);
                compare_results("runQ14Query", "Line", result_ELF_subtree);
                cout << "Node level" << endl;
                result_ELF_nodelevel_enqueue = runQ14Query(elfFinal64_nodelevel_enqueue, partialsTPCH4);
                compare_results("runQ14Query", "Line", result_ELF_nodelevel_enqueue);
                clear_vectors();
                cout << endl;

                cout << "Sequential Elf" << endl;
                result_ELF = runQ19Query(elfFinal64, partialsTPCH5, partialsTPCH6, partialsTPCH7);
                cout << "Sub-tree" << endl;
                result_ELF_subtree = runQ19Query(elfFinal64_subtree, partialsTPCH5, partialsTPCH6, partialsTPCH7);
                compare_results("runQ19Query", "Line", result_ELF_subtree);
                cout << "Node level" << endl;
                result_ELF_nodelevel_enqueue = runQ19Query(elfFinal64_nodelevel_enqueue, partialsTPCH5, partialsTPCH6, partialsTPCH7);
                compare_results("runQ19Query", "Line", result_ELF_nodelevel_enqueue);
                clear_vectors();
                cout << endl;

                /*runQ6Query(elfFinal64, partialsTPCH2);
                runQ10Query(elfFinal64, partialsTPCH3);
                runQ14Query(elfFinal64, partialsTPCH4);
                runQ19Query(elfFinal64, partialsTPCH5, partialsTPCH6, partialsTPCH7);*/

            }

#endif
#ifdef Part
        for (int i = 1 ; i <= thread_limit ; i = i + 1) {
            clear_vectors();
            cout << endl;

            cout << "Running for threads " << i << endl;
            elfFinal64_subtree->setNumThreads(i);
            elfFinal64_nodelevel_enqueue->setNumThreads(i);
            cout << endl;

            cout << "Sequential Elf" << endl;
            result_ELF = runQ17Query(elfFinal64, partialsTPCH1);
            cout << "Sub-tree" << endl;
            result_ELF_subtree = runQ17Query(elfFinal64_subtree, partialsTPCH1);
            compare_results("runQ17Query", "Line", result_ELF_subtree);
            cout << "Node level" << endl;
            result_ELF_nodelevel_enqueue = runQ17Query(elfFinal64_nodelevel_enqueue, partialsTPCH1);
            compare_results("runQ17Query", "Line", result_ELF_nodelevel_enqueue);
            clear_vectors();
            cout << endl;


            cout << "Sequential Elf" << endl;
            result_ELF = runQ19Query(elfFinal64, partialsTPCH2, partialsTPCH3, partialsTPCH4);
            cout << "Sub-tree" << endl;
            result_ELF_subtree = runQ19Query(elfFinal64_subtree, partialsTPCH2, partialsTPCH3, partialsTPCH4);
            compare_results("runQ19Query", "Line", result_ELF_subtree);
            cout << "Node level" << endl;
            result_ELF_nodelevel_enqueue = runQ19Query(elfFinal64_subtree, partialsTPCH2, partialsTPCH3, partialsTPCH4);
            compare_results("runQ19Query", "Line", result_ELF_nodelevel_enqueue);
            clear_vectors();
            cout << endl;

        }

        cout << endl;
#endif
//        delete (Elf_final64*) elfFinal64;
#endif




    }
    delete s;
    result_ELF.clear();
    result.clear();
    result_difference.clear();
    //    	for(uint32_t dim=0;dim<numDim;dim++){
    //            delete [] data[dim];
    //    	}
    delete [] data;

    cout << "Ready!" << endl;
    return 0;
}
