#pragma once
/*
* introSort.h
*
*  Created on: Sep 27, 2015
*      Author: ArchiDave
*/

#ifndef INTROSORT_HPP_
#define INTROSORT_HPP_

#include "sortingEngineUtils.h"
using namespace std;


//template< class T >
//void _doheap(TID<T>* data, int begin, int end, uint32_t dim, uint32_t size)
//{
//        //TODO: implement _swap here for TID and values
//        TID<T> save;
//	save.tid = data[begin].tid;
//#ifdef PointerSwap
//        save.value = data[begin].value;
//#else
//        save.value=(T*)malloc(sizeof(T)*(size-dim));
//        memcpy(save.value,&(data[begin].value[dim]),sizeof(T)*(size-dim));
//#endif
//	while (begin <= end / 2) {
//		int k = 2 * begin;
//
//		while (k < end && data[k].value[dim] <data[k + 1].value[dim])
//			++k;
//
//		if (save.value[dim] >= data[k].value[dim])
//			break;
//		data[begin].tid = data[k].tid;
//#ifdef PointerSwap
//        data[begin].value = data[k].value;
//#else
//                memcpy(&(data[begin].value[dim]),&(data[k].value[dim]),sizeof(T)*(size-dim));
//#endif
//		begin = k;
//	}
//
//	data[begin].tid = save.tid;
//#ifdef PointerSwap
//        data[begin].value = save.value;
//#else        
//        memcpy(&(data[begin].value[dim]), save.value, sizeof(T)*(size-dim));
//        free (save.value);
//#endif
//}

//template< class T >
//void _heapsort(TID<T>* data, int begin, int end, uint32_t dim, uint32_t size)
//{
//	int i;
//
//	for (int i = (end - 1) / 2; i >= begin; i--) {
//		_doheap(data, i, end - 1,  dim, size);
//	}
//
//	for (i = end - 1; i>begin; i--) {
//		_swap(data[i], data[begin], dim, size);
//		_doheap(data, begin, i - 1, dim, size);
//	}
//}

template< class T >
void introsort_r(TID<T>* data, long first, long last, long depth, uint32_t dim, uint32_t size)
{
	//if last or first contains <16 elements -> switch to insertion sort
	while (last - first > 0) {
		if (depth == 0){
			_heapsort(&data[first], 0, last - first + 1, dim, size);
                        break;
		} else {
			int pivot;
			if (_isSorted(data, last, dim))
				break;
			pivot = _partition(data, first, last, dim, size);
			introsort_r(data, pivot + 1, last, depth - 1, dim, size);
			last = pivot - 1;
		}
	}
}

template< class T >
void insertion(TID<T>* data, TID<T>& save, long n, uint32_t dim, uint32_t size)
{
	int i;
	for (i = 1; i < n; i++) {
        //TODO: implement _swap here for TID and values
            save.tid = data[i].tid;
#ifdef PointerSwap
        save.value = data[i].value;
#else           
            memcpy(save.value,&(data[i].value[dim]),sizeof(T)*(size-dim));
#endif
            uint32_t j;

		for (j = i; j >= 1 && data[j - 1].value[dim] > save.value[0]; j--){
			data[j].tid = data[j - 1].tid;
#ifdef PointerSwap
        data[j].value = data[j-1].value;
#else
                        memcpy(&(data[j].value[dim]),&(data[j - 1].value[dim]),sizeof(T)*(size-dim));
#endif
                      
                }

		data[j].tid = save.tid;
 #ifdef PointerSwap
        data[j].value = save.value;
#else
                memcpy(&(data[j].value[dim]),save.value,sizeof(T)*(size-dim));
#endif
	}
}

template< class T >
void introsort(TID<T>* data,TID<T>& save, size_t n, uint32_t dim, uint32_t size)
{
	//introsort_r(data, 0, n - 1, (long(2 * log(double(n)))), dim, size);
	insertion(data, save, n, dim, size);
}


#endif /* INTROSORT_HPP_ */
