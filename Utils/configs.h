#pragma once

/*
* configs.h
*
* Represents a configuration file for B-MPSM
*
*
*  Created on: Sep 27, 2015
*      Author: ArchiDave
*/

#ifndef CONFIGS_HPP_
#define CONFIGS_HPP_

//#include <boost/thread.hpp>

//#define DEBUG

#define M_NUMBER_OF_BINS 256	
#define M_LOG2_OF_NUMBER_OF_BINS 8
#define M_INTROSORT_SWITCH 100
#define M_RANDOM_VALUES_COUNT 10000000
#define M_RANDOM_VALUES_LIMIT 10000

#define SMART_SHIFTER
//#define RADIX_CHECK
//#define NEW_MEDIAN

/*
* Number of worker threads used in the system
*/
/*const int NUMBER_OF_WORKER_THREADS = boost::thread::hardware_concurrency();*/

/*
* defines a number of bins for radix sort
*
* !!!has to be a Power of 2!!!
* (default = 256)
*/
const unsigned long NUMBER_OF_BINS = M_NUMBER_OF_BINS;
/* LOG2 of NUMBER_OF_BINS */
const unsigned long LOG2_OF_NUMBER_OF_BINS = M_LOG2_OF_NUMBER_OF_BINS;

/*
* defines a limit of entries in the bin
* for switching to IntroSort after breach
* (default = 16)
*/
const unsigned long INTROSORT_SWITCH = M_INTROSORT_SWITCH;

/*
* presets for defining random values
* (for MPSM Simulator)
*/
const size_t RANDOM_VALUES_COUNT = M_RANDOM_VALUES_COUNT;
const unsigned int RANDOM_VALUES_LIMIT = M_RANDOM_VALUES_LIMIT;

/*
* activate/deactivate test mode,
* which compares the result with nested loop
*/
const bool TEST_MOD = true;

/*
	LOG LEVEL
		0 : no logs
		1 : basics
		2 : detailed information
*/
const int LOG_LEVEL = 0;


#endif /* CONFIGS_HPP_ */
