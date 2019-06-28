#pragma once

#ifndef SORTINGENGINEUTILS_HPP_
#define SORTINGENGINEUTILS_HPP_

#include "../Utils/structures.h"
//template< class _Type >
//_Type _partition ( _Type* a, int first, int last);
//template< class _Type >
//void _swap(_Type *a, _Type *b);
//template< class _Type >
//bool _isSorted(_Type* a, long end);

template< class T >
void _swap(TID<T>& a, TID<T>& b, TID<T>& save, uint32_t dim, uint32_t size)
{       
//        TID<T> save;
#ifdef PointerSwap
	save.tid = a.tid;
        save.value = a.value;
        a.tid = b.tid;
        a.value = b.value;
        b.tid = save.tid;
        b.value = save.value;
    
#else
	save.tid = a.tid;
        //save.value=(T*)malloc(sizeof(T)*(size-dim));
        memcpy(save.value,&(a.value[dim]),sizeof(T)*(size-dim));
	a.tid = b.tid;
        memcpy(&(a.value[dim]), &(b.value[dim]), sizeof(T)*(size-dim));
	b.tid = save.tid;
        memcpy(&(b.value[dim]), save.value, sizeof(T)*(size-dim));
        //free(save.value);
#endif
        
}

template< class T >
int _partition(TID<T>* a, size_t first, size_t last, uint32_t dim, uint32_t size)
{
	size_t pivot;
	size_t mid = (first + last) / 2;

#ifdef NEW_MEDIAN
	if(a[first].value > a[mid].value)
		_swap(a[first], a[mid], dim, size);
	if(a[first].value > a[last].value)
		_swap(a[first], a[last], dim, size);
	else if (a[last].value > a[mid].value)
		_swap(a[last], a[mid], dim, size);

#else	
	pivot = a[first].value[dim] > a[mid].value[dim] ? first : mid;

	if (a[pivot].value[dim]  > a[last].value[dim] )
		pivot = last;

	_swap(a[pivot], a[first], dim, size);
#endif // NEW_MEDIAN

	pivot = first;

	while (first < last) {
		if (a[first].value[dim]  <= a[last].value[dim])
			_swap(a[pivot++], a[first], dim, size);
		++first;
	}

	_swap(a[pivot], a[last], dim, size);
	return pivot;
}

template< class T >
bool _isSorted(TID<T>* a, int end, uint32_t dim)
{
	for (int i = 0; i < end; i++) {
		if (a[i].value[dim] > a[i + 1].value[dim]) {
			return false;
		}
	}
	return true;
}

#endif /* SORTINGENGINEUTILS_HPP_ */