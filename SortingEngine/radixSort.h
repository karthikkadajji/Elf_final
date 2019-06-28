#pragma once

#ifndef RADIXSORT_HPP_
#define RADIXSORT_HPP_

#include "introSort.h"
#include "sortingEngineUtils.h"

template< unsigned long PowerOfTwoRadix, unsigned long Log2ofNumberOfBins, long INTROSORT_SWITCH, class T >
void radixSort(TID<T>* data ,TID<T>& save, size_t last, T mask, unsigned long shifter,uint32_t dim, uint32_t size)
{
	#ifdef DEBUG
	cout << "STARTED" << endl;
	#endif
	const unsigned long NumberOfBins = PowerOfTwoRadix;
	unsigned long count[NumberOfBins];

	// Counting occurrence of digits within the array
	for (unsigned long i = 0; i < NumberOfBins; i++)
		count[i] = 0;

	// Scan the array and count the number of times each value appears
	for (size_t _current = 0; _current <= last; _current++)
	{
		// extract the digit we are sorting based on
		unsigned long digit = (unsigned long)((data[_current].value[dim] & mask) >> shifter);
		count[digit]++;
	}

	// Moving array elements into their bins
	size_t startOfBin[NumberOfBins], endOfBin[NumberOfBins], nextBin;
	startOfBin[0] = endOfBin[0] = nextBin = 0;
	for (unsigned long i = 1; i < NumberOfBins; i++)
		startOfBin[i] = endOfBin[i] = startOfBin[i - 1] + count[i - 1];

	for (size_t _current = 0; _current <= last; )
	{
		unsigned long digit;
		TID<T> tid = data[_current]; // get the compiler to recognize that a register can be used for the loop instead of a[_current] memory location
		#ifdef DEBUG
		cout << "tid: [" << tid.tid <<","<<tid.value[dim]<<"]"<< endl;
		#endif
		while (true) {
			digit = (unsigned long)((tid.value[dim] & mask) >> shifter);  // extract the digit we are sorting based on;
			#ifdef DEBUG
			cout << "digit: " << digit << endl;
			cout << "endOfBin[digit]: " << endOfBin[digit] << endl;
			cout << "_current: " << _current << endl;
			#endif
			if (endOfBin[digit] == _current)
				break;
			//takes much longer with _swap
			//_swap(tid, data[endOfBin[digit]]);
			#ifdef DEBUG
			cout << "swap: " << endl;
			#endif
			
#ifdef RADIX_CHECK
			//don't need to swap by equal values
			if (tid.value[dim] != data[endOfBin[digit]].value[dim]) {
				_swap(tid, data[endOfBin[digit]]);
			}
#else
			_swap(tid,  data[endOfBin[digit]],save, dim, size);
#endif // RADIX_CHECK

			endOfBin[digit]++;
		}
		data[_current]= tid;

		endOfBin[digit]++;                    // leave the element at its location and grow the bin
		_current++;                             // advance the current pointer to the next element
		while (_current >= startOfBin[nextBin] && nextBin < NumberOfBins)
			nextBin++;
		while (endOfBin[nextBin - 1] == startOfBin[nextBin] && nextBin < NumberOfBins)
			nextBin++;
		if (_current < endOfBin[nextBin - 1])
			_current = endOfBin[nextBin - 1];
	}
	// Recursion for each bin
	mask >>= Log2ofNumberOfBins;
	if (mask != 0)                     // end recursion when all the bits have been processes
	{
		if (shifter >= Log2ofNumberOfBins) 
			shifter -= Log2ofNumberOfBins;
		else 
			shifter = 0;

		for (unsigned long i = 0; i < NumberOfBins; i++)
		{
			long numberOfElements = endOfBin[i] - startOfBin[i];
			if (numberOfElements >= INTROSORT_SWITCH) {       // endOfBin actually points to one beyond the bin
				radixSort<NumberOfBins, Log2ofNumberOfBins, INTROSORT_SWITCH, T>(&data[startOfBin[i]], save, numberOfElements - 1, mask, shifter, dim, size);
			}
			else if (numberOfElements >= 2) {
				introsort(&data[startOfBin[i]],save, numberOfElements, dim, size);
			}
		}
	}
}


#endif /* RADIXSORT_HPP_ */