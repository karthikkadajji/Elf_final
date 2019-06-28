#pragma once


#ifndef SORTINGENGINE_HPP_
#define SORTINGENGINE_HPP_
#include "math.h"
#include <iostream>

#include "../Utils/configs.h"

#include "radixSort.h"

#include "introSort.h"

using namespace std;

template< class T >
void sort(Array<T>* data_array, int dim, uint32_t* value_ranges, uint32_t size) {
    TID<T> save;
    save.value=(T*)malloc(sizeof(T)*size);
    size_t data_size = data_array->size;
    TID<T> *data = data_array->ary;
    if (data_size < 2)
        return;

    if (data_size >= INTROSORT_SWITCH) {

        unsigned long shifter;

#ifdef SMART_SHIFTER
        unsigned long r = 1;
        unsigned long limit = value_ranges[dim];
        while (limit >>= 1) r++;
        shifter = (r / LOG2_OF_NUMBER_OF_BINS) * LOG2_OF_NUMBER_OF_BINS;
#else
        shifter = bitsnumber - LOG2_OF_NUMBER_OF_BINS;
#endif // SMART_SHIFTER

        //create shifter and mask based on data in configs
        T mask = (T) (((T) (NUMBER_OF_BINS - 1)) << shifter);
        radixSort< NUMBER_OF_BINS, LOG2_OF_NUMBER_OF_BINS, INTROSORT_SWITCH >(data, save, data_size - 1, mask, shifter, dim, size);

    } else {
        introsort(data, save, data_size, dim, size);
    }
}



#endif /* SORTINGENGINE_HPP_ */