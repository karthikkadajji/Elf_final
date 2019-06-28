#pragma once

#ifndef STRUCTURES_H_
#define STRUCTURES_H_


#include <stddef.h>

template< typename T >
struct Result {
	T privateTID;
	T publicTID;
	T value;
};

template< typename T >
struct RandomParts {
	T* privatePart;
	T* publicPart;
	size_t privatePart_size;
	size_t publicPart_size;
};

/*	element struct, contains position and actual value  */
template<typename T>
struct TID {
	T tid;
	T *value;
};

/*	Array of TID structs with array size  */
template< typename T>
struct Array {
	TID<T> *ary;
	size_t size;
};


/*	Representation of Elements pro Worker  */
template< typename T >
struct Parts {
	Array<T> privatePart;
	Array<T> publicPart;
	size_t maxLength;
};

/*	Basic Structure,contains everything  */
template< class T >
class Structure {
public:
	Parts<T> * parts;
	size_t maxLength;
	Structure(T*, T*, size_t, size_t);
};

#endif /* STRUCTURES_H_ */