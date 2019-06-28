#ifndef CONFIG_H
#define CONFIG_H

//#define NO_SIMD_OPERATIONS 1

//Index Structures

//#define SEQ 1
#define COLSEQ 1


//#define SIMDSeq 1

//#define Imprint 1
//#define ImprintWC 1

//#define ImprintWCSIMD 1
//#define ImprintWCSIMD64 1

//#define KDTreeSmall 1

//#define BITWEAVINGV 1
//#define BITWEAVINGH 1


//#define SortedProj 1


#define ElfFinalBigPtr 1

//needs to be included for Testing the Elf_7
//#define REDUCED_DIM 1

//Choose right
//#define LineItem
#define Part
// 100 Partial Ma
// tches

#define NUM_Exact 1
#define NUM_Partials 9
#define NUM_Range 9

#define NUMREPS 1

//#define BW_TWO_PASS 1
//Change unit test
#define UnitTest 1
#define ORDER_Q6

#endif // CONFIG_H
