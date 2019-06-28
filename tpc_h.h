#ifndef TPC_H_H
#define TPC_H_H


#include <fstream>


#include "Config.h"
#include "Util.h"

#define RUN_TPC_H

#ifdef RUN_TPC_H

    #ifdef ORDER_A//only one query
	
		uint32_t COLUMN_TO_UPDATE = 9;
		uint32_t DATE_COLUMN = 9;
		uint32_t DISCOUNT_COLUMN = 6;
		uint32_t QUANTITY_COLUMN = 7;
		uint32_t LINESTATUS = 0;
		uint32_t RETURNFLAG = 1;
		uint32_t SHIPINSTR = 2;
#elif defined ORDER_Q6
uint32_t COLUMN_TO_UPDATE = 0;
uint32_t DATE_COLUMN = 0;
uint32_t DISCOUNT_COLUMN = 1;
uint32_t QUANTITY_COLUMN = 2;
uint32_t LINESTATUS = 3;
uint32_t RETURNFLAG = 4;
uint32_t SHIPINSTR = 5;

#elif defined ORDER_Q6Opt
uint32_t COLUMN_TO_UPDATE = 2;
uint32_t DATE_COLUMN = 2;
uint32_t DISCOUNT_COLUMN = 0;
uint32_t QUANTITY_COLUMN = 1;
uint32_t LINESTATUS = 3;
uint32_t RETURNFLAG = 4;
uint32_t SHIPINSTR = 5;

	#endif // RUN_QUERY_1
	#define QUERY
#endif //RUN_TPC_H

/**
  * Date 01.12.1998 has 2525 as compressed value
  * Using OrderA this correponds to DIM 10, using Order B Order Q6 Dim 1 valid ranges are thus [0,RETURN_VALUE]
  * valid return values are:
  * 90 days earlier = 02.09.1998 = 	2435
  * 60 days earlier = 02.10.1998 = 	2465
  * 120 days earlier = 03.08.1998 = 2405
  */
uint32_t generateQ1EndDate(){
	uint32_t random = rand()%120;
    return random+2406;
}

void updateQ1(PartialMatchQueries* ranges){
	for(uint32_t query=0;query<ranges->NUM_QUERIES;query++){

		for(uint32_t dim=0;dim<ranges->NUM_DIM;dim++){
			ranges->lowerBound[query][dim] = 0;
			ranges->upperBound[query][dim] = 0;
		}
		ranges->upperBound[query][COLUMN_TO_UPDATE] = generateQ1EndDate();
	}

        // Update Wildcards!
	for(uint32_t dim=0;dim<ranges->NUM_DIM;dim++){
		ranges->columnsForSelect[dim] = 0;
	}

	ranges->columnsForSelect[COLUMN_TO_UPDATE] = 1;
}

void updateQ6(PartialMatchQueries* ranges){
	uint32_t random;

	for(uint32_t query=0;query<ranges->NUM_QUERIES;query++){

		for(uint32_t dim=0;dim<ranges->NUM_DIM;dim++){
			ranges->lowerBound[query][dim] = 0;
			ranges->upperBound[query][dim] = 0;
		}
		//*****************BEGIN DATE
		/* Possible values for start date
			0: 01-01-1992:    0
			1: 01-01-1993:  365
			2: 01-01-1994:  730
			3: 01-01-1995: 1095
			4: 01-01-1996: 1460
			5: 01-01-1997: 1826
			-: 01-01-1998: 2191
		*/
		random = rand()%6;//index start date

		if(random==0){
			ranges->lowerBound[query][DATE_COLUMN]=0;
			ranges->upperBound[query][DATE_COLUMN]=365-1;
		}else if(random==1){
			ranges->lowerBound[query][DATE_COLUMN]=365;
			ranges->upperBound[query][DATE_COLUMN]=730-1;
		}else if(random==2){
			ranges->lowerBound[query][DATE_COLUMN]=730;
			ranges->upperBound[query][DATE_COLUMN]=1095-1;
		}else if(random==3){
			ranges->lowerBound[query][DATE_COLUMN]=1095;
			ranges->upperBound[query][DATE_COLUMN]=1460-1;
		}else if(random==4){
			ranges->lowerBound[query][DATE_COLUMN]=1460;
			ranges->upperBound[query][DATE_COLUMN]=1826-1;
		}else{ // == 5
			ranges->lowerBound[query][DATE_COLUMN]=1826;
			ranges->upperBound[query][DATE_COLUMN]=2191-1;
		}
		//*****************END DATE

		//*****************BEGIN Discount
		/* Possible values for discount interval
			[1, 3];
			[2, 4];
			[3, 5];
			[4, 6];
			[5, 7];
			[6, 8];
			[7, 9];
			[8, 10];
		*/
		random = (rand()%8)+1;
		ranges->lowerBound[query][DISCOUNT_COLUMN]=random;
		ranges->upperBound[query][DISCOUNT_COLUMN]=random+2;
		//*****************END Discount

		//*****************BEGIN Quantity
		/* Possible values for discount interval
			[0, 24];
			[0, 25];
		*/
		random = rand()%2;
		if(random==0){
			ranges->upperBound[query][QUANTITY_COLUMN] = 24;
		}else{
			ranges->upperBound[query][QUANTITY_COLUMN] = 25;
		}
		//*****************END Quantity
         }
        // Update Wildcards!
	for(uint32_t dim=0;dim<ranges->NUM_DIM;dim++){
		ranges->columnsForSelect[dim] = 0;
	}

	
	ranges->columnsForSelect[DATE_COLUMN] = 1;
	ranges->columnsForSelect[DISCOUNT_COLUMN] = 1;
	ranges->columnsForSelect[QUANTITY_COLUMN] = 1;
	
}


void updateQ10(PartialMatchQueries* ranges){
	for(uint32_t query=0;query<ranges->NUM_QUERIES;query++){

		for(uint32_t dim=0;dim<ranges->NUM_DIM;dim++){
			ranges->lowerBound[query][dim] = 0;
			ranges->upperBound[query][dim] = 0;
		}
		ranges->lowerBound[query][4] = 2;
		ranges->upperBound[query][4] = 2;
	}

        // Update Wildcards!
	for(uint32_t dim=0;dim<ranges->NUM_DIM;dim++){
		ranges->columnsForSelect[dim] = 0;
	}

	ranges->columnsForSelect[4] = 1;
}


void updateQ14(PartialMatchQueries* ranges){
	for(uint32_t query=0;query<ranges->NUM_QUERIES;query++){

		for(uint32_t dim=0;dim<ranges->NUM_DIM;dim++){
			ranges->lowerBound[query][dim] = 0;
			ranges->upperBound[query][dim] = 0;
		}
	

        int year = rand()%5;//index start date
        int month = rand()%12;//index start date
        int beginMonth = 0;
        int endMonth = 31;
        if(month>0){
            beginMonth+=31;
            endMonth=28;
        }
        if(month>1){
            beginMonth+=28;
            endMonth=31;
        }
        if(month>2){
            beginMonth+=31;
            endMonth=30;
        }
        if(month>3){
            beginMonth+=30;
            endMonth=31;
        }
        if(month>4){
            beginMonth+=31;
            endMonth=30;
        }
        if(month>5){
            beginMonth+=30;
            endMonth=31;
        }
        if(month>6){
            beginMonth+=31;
            endMonth=31;
        }
        if(month>7){
            beginMonth+=31;
            endMonth=30;
        }
        if(month>8){
            beginMonth+=30;
            endMonth=31;
        }
        if(month>9){
            beginMonth+=31;
            endMonth=30;
        }
        if(month>10){
            beginMonth+=30;
            endMonth=31;
        }
        //cout << "Month: " << month << " BeginMonth: "<< beginMonth << endl;
        ranges->lowerBound[query][DATE_COLUMN]=365+year*365+beginMonth;
        ranges->upperBound[query][DATE_COLUMN]=365+year*365+beginMonth+endMonth;
        }
        // Update Wildcards!
	for(uint32_t dim=0;dim<ranges->NUM_DIM;dim++){
		ranges->columnsForSelect[dim] = 0;
	}

	ranges->columnsForSelect[DATE_COLUMN] = 1;
}

void updateQ17(PartialMatchQueries* ranges){
    uint32_t random;
    
    for(uint32_t query=0;query<ranges->NUM_QUERIES;query++){
        
        for(uint32_t dim=0;dim<ranges->NUM_DIM;dim++){
            ranges->lowerBound[query][dim] = 0;
            ranges->upperBound[query][dim] = 0;
        }
        //*****************BEGIN BRAND
        
        random = rand()%25;//index start date
        ranges->lowerBound[query][1]=random;
        ranges->upperBound[query][1]=random;
        //*****************END BRAND
        
        //*****************BEGIN CONTAINER

        random = rand()%40;
        ranges->lowerBound[query][2]=random;
        ranges->upperBound[query][2]=random;
        //*****************END CONTAINER
        
     
    }
    // Update Wildcards!
    for(uint32_t dim=0;dim<ranges->NUM_DIM;dim++){
        ranges->columnsForSelect[dim] = 0;
    }
    
    
    ranges->columnsForSelect[1] = 1;
    ranges->columnsForSelect[2] = 1;
    
}

void updateQ19_1(PartialMatchQueries* ranges){
    uint32_t random;
    
    for(uint32_t query=0;query<ranges->NUM_QUERIES;query++){
        
        for(uint32_t dim=0;dim<ranges->NUM_DIM;dim++){
            ranges->lowerBound[query][dim] = 0;
            ranges->upperBound[query][dim] = 0;
        }
        //*****************BEGIN BRAND
        
        random = rand()%25;//index start date
        ranges->lowerBound[query][1]=random;
        ranges->upperBound[query][1]=random;
        //*****************END BRAND
        
        //*****************BEGIN CONTAINER
        ranges->lowerBound[query][2]=25;
        ranges->upperBound[query][2]=28;
        //*****************END CONTAINER
        
        //*****************BEGIN Quantity
        ranges->lowerBound[query][3]=0;
        ranges->upperBound[query][3]=4;
        //*****************END Quantity
    }
    // Update Wildcards!
    for(uint32_t dim=0;dim<ranges->NUM_DIM;dim++){
        ranges->columnsForSelect[dim] = 0;
    }
    
    
    ranges->columnsForSelect[1] = 1;
    ranges->columnsForSelect[2] = 1;
    ranges->columnsForSelect[3] = 1;
    
}

void updateQ19_2(PartialMatchQueries* ranges){
    uint32_t random;
    
    for(uint32_t query=0;query<ranges->NUM_QUERIES;query++){
        
        for(uint32_t dim=0;dim<ranges->NUM_DIM;dim++){
            ranges->lowerBound[query][dim] = 0;
            ranges->upperBound[query][dim] = 0;
        }
        //*****************BEGIN BRAND
        
        random = rand()%25;//index start date
        ranges->lowerBound[query][1]=random;
        ranges->upperBound[query][1]=random;
        //*****************END BRAND
        
        //*****************BEGIN CONTAINER
        ranges->lowerBound[query][2]=16;
        ranges->upperBound[query][2]=19;
        //*****************END CONTAINER
        
        //*****************BEGIN Quantity
        ranges->lowerBound[query][3]=0;
        ranges->upperBound[query][3]=9;
        //*****************END Quantity
    }
    // Update Wildcards!
    for(uint32_t dim=0;dim<ranges->NUM_DIM;dim++){
        ranges->columnsForSelect[dim] = 0;
    }
    
    
    ranges->columnsForSelect[1] = 1;
    ranges->columnsForSelect[2] = 1;
    ranges->columnsForSelect[3] = 1;
    
}

void updateQ19_3(PartialMatchQueries* ranges){
    uint32_t random;
    
    for(uint32_t query=0;query<ranges->NUM_QUERIES;query++){
        
        for(uint32_t dim=0;dim<ranges->NUM_DIM;dim++){
            ranges->lowerBound[query][dim] = 0;
            ranges->upperBound[query][dim] = 0;
        }
        //*****************BEGIN BRAND
        
        random = rand()%25;//index start date
        ranges->lowerBound[query][1]=random;
        ranges->upperBound[query][1]=random;
        //*****************END BRAND
        
        //*****************BEGIN CONTAINER
        ranges->lowerBound[query][2]=9;
        ranges->upperBound[query][2]=12;
        //*****************END CONTAINER
        
        //*****************BEGIN Quantity
        ranges->lowerBound[query][3]=0;
        ranges->upperBound[query][3]=14;
        //*****************END Quantity
    }
    // Update Wildcards!
    for(uint32_t dim=0;dim<ranges->NUM_DIM;dim++){
        ranges->columnsForSelect[dim] = 0;
    }
    
    
    ranges->columnsForSelect[1] = 1;
    ranges->columnsForSelect[2] = 1;
    ranges->columnsForSelect[3] = 1;
    
}

void updateLQ19_1(PartialMatchQueries* ranges){
    uint32_t random;
    
    for(uint32_t query=0;query<ranges->NUM_QUERIES;query++){
        
        for(uint32_t dim=0;dim<ranges->NUM_DIM;dim++){
            ranges->lowerBound[query][dim] = 0;
            ranges->upperBound[query][dim] = 0;
        }
        //*****************BEGIN Quantity
        
        random = rand()%10;//index start date
        ranges->lowerBound[query][2]=random;
        ranges->upperBound[query][2]=random+9;
        //*****************END Quantity
        
        //*****************BEGIN Shipinstr
        ranges->lowerBound[query][5]=1;
        ranges->upperBound[query][5]=1;
        //*****************END Shipinstr
        
        //*****************BEGIN Shipmode
        ranges->lowerBound[query][6]=0;
        ranges->upperBound[query][6]=1;
        //*****************END Shipmode
        
    }
    // Update Wildcards!
    for(uint32_t dim=0;dim<ranges->NUM_DIM;dim++){
        ranges->columnsForSelect[dim] = 0;
    }
    
    
    ranges->columnsForSelect[2] = 1;
    ranges->columnsForSelect[5] = 1;
    ranges->columnsForSelect[6] = 1;
    
}
void updateLQ19_2(PartialMatchQueries* ranges){
    uint32_t random;
    
    for(uint32_t query=0;query<ranges->NUM_QUERIES;query++){
        
        for(uint32_t dim=0;dim<ranges->NUM_DIM;dim++){
            ranges->lowerBound[query][dim] = 0;
            ranges->upperBound[query][dim] = 0;
        }
        //*****************BEGIN Quantity
        
        random = rand()%10;//index start date
        ranges->lowerBound[query][2]=random+10;
        ranges->upperBound[query][2]=random+19;
        //*****************END Quantity
        
        //*****************BEGIN Shipinstr
        ranges->lowerBound[query][5]=1;
        ranges->upperBound[query][5]=1;
        //*****************END Shipinstr
        
        //*****************BEGIN Shipmode
        ranges->lowerBound[query][6]=0;
        ranges->upperBound[query][6]=1;
        //*****************END Shipmode
        
    }
    // Update Wildcards!
    for(uint32_t dim=0;dim<ranges->NUM_DIM;dim++){
        ranges->columnsForSelect[dim] = 0;
    }
    
    
    ranges->columnsForSelect[2] = 1;
    ranges->columnsForSelect[5] = 1;
    ranges->columnsForSelect[6] = 1;
    
}
void updateLQ19_3(PartialMatchQueries* ranges){
    uint32_t random;
    
    for(uint32_t query=0;query<ranges->NUM_QUERIES;query++){
        
        for(uint32_t dim=0;dim<ranges->NUM_DIM;dim++){
            ranges->lowerBound[query][dim] = 0;
            ranges->upperBound[query][dim] = 0;
        }
        //*****************BEGIN Quantity
        
        random = rand()%10;//index start date
        ranges->lowerBound[query][2]=random+20;
        ranges->upperBound[query][2]=random+29;
        //*****************END Quantity
        
        //*****************BEGIN Shipinstr
        ranges->lowerBound[query][5]=1;
        ranges->upperBound[query][5]=1;
        //*****************END Shipinstr
        
        //*****************BEGIN Shipmode
        ranges->lowerBound[query][6]=0;
        ranges->upperBound[query][6]=1;
        //*****************END Shipmode
        
    }
    // Update Wildcards!
    for(uint32_t dim=0;dim<ranges->NUM_DIM;dim++){
        ranges->columnsForSelect[dim] = 0;
    }
    
    
    ranges->columnsForSelect[2] = 1;
    ranges->columnsForSelect[5] = 1;
    ranges->columnsForSelect[6] = 1;
    
}



void updateVeit(PartialMatchQueries* ranges){
	uint32_t random;

	for(uint32_t query=0;query<ranges->NUM_QUERIES;query++){

		for(uint32_t dim=0;dim<ranges->NUM_DIM;dim++){
			ranges->lowerBound[query][dim] = 0;
			ranges->upperBound[query][dim] = 0;
		}
		//*****************BEGIN DATE
		/* Possible values for start date
			0: 01-01-1992:    0
			1: 01-01-1993:  365
			2: 01-01-1994:  730
			3: 01-01-1995: 1095
			4: 01-01-1996: 1460
			5: 01-01-1997: 1826
			-: 01-01-1998: 2191
		*/
		random = rand()%6;//index start date

		if(random==0){
			ranges->lowerBound[query][DATE_COLUMN]=0;
			ranges->upperBound[query][DATE_COLUMN]=365-1;
		}else if(random==1){
			ranges->lowerBound[query][DATE_COLUMN]=365;
			ranges->upperBound[query][DATE_COLUMN]=730-1;
		}else if(random==2){
			ranges->lowerBound[query][DATE_COLUMN]=730;
			ranges->upperBound[query][DATE_COLUMN]=1095-1;
		}else if(random==3){
			ranges->lowerBound[query][DATE_COLUMN]=1095;
			ranges->upperBound[query][DATE_COLUMN]=1460-1;
		}else if(random==4){
			ranges->lowerBound[query][DATE_COLUMN]=1460;
			ranges->upperBound[query][DATE_COLUMN]=1826-1;
		}else{ // == 5
			ranges->lowerBound[query][DATE_COLUMN]=1826;
			ranges->upperBound[query][DATE_COLUMN]=2191-1;
		}
		//*****************END DATE

		//*****************BEGIN Discount
		/* Possible values for discount interval
			[1, 3];
			[2, 4];
			[3, 5];
			[4, 6];
			[5, 7];
			[6, 8];
			[7, 9];
			[8, 10];
		*/
		random = (rand()%8)+1;
		ranges->lowerBound[query][DISCOUNT_COLUMN]=random;
		ranges->upperBound[query][DISCOUNT_COLUMN]=random+2;
		//*****************END Discount

		//*****************BEGIN Quantity
		/* Possible values for discount interval
			[0, 24];
			[0, 25];
		*/
		random = rand()%2;
		if(random==0){
			ranges->upperBound[query][QUANTITY_COLUMN] = 24;
		}else{
			ranges->upperBound[query][QUANTITY_COLUMN] = 25;
		}
		//*****************END Quantity


		//*****************BEGIN Linestatus
		/* Possible values for discount interval
			[0, 24];
			[0, 25];
		*/
		random = rand()%2;
		
		ranges->lowerBound[query][LINESTATUS]=random;
		ranges->upperBound[query][LINESTATUS]=random;
		//*****************END Quantity
	}
// Update Wildcards!
	for(uint32_t dim=0;dim<ranges->NUM_DIM;dim++){
		ranges->columnsForSelect[dim] = 0;
	}

	
	ranges->columnsForSelect[DATE_COLUMN] = 1;
	ranges->columnsForSelect[DISCOUNT_COLUMN] = 1;
	ranges->columnsForSelect[QUANTITY_COLUMN] = 1;
	ranges->columnsForSelect[LINESTATUS] = 1;
	
}




void updateVeit5(PartialMatchQueries* ranges){
	uint32_t random;

	for(uint32_t query=0;query<ranges->NUM_QUERIES;query++){

		for(uint32_t dim=0;dim<ranges->NUM_DIM;dim++){
			ranges->lowerBound[query][dim] = 0;
			ranges->upperBound[query][dim] = 0;
		}
		//*****************BEGIN DATE
		/* Possible values for start date
			0: 01-01-1992:    0
			1: 01-01-1993:  365
			2: 01-01-1994:  730
			3: 01-01-1995: 1095
			4: 01-01-1996: 1460
			5: 01-01-1997: 1826
			-: 01-01-1998: 2191
		*/
		random = rand()%6;//index start date

		if(random==0){
			ranges->lowerBound[query][DATE_COLUMN]=0;
			ranges->upperBound[query][DATE_COLUMN]=365-1;
		}else if(random==1){
			ranges->lowerBound[query][DATE_COLUMN]=365;
			ranges->upperBound[query][DATE_COLUMN]=730-1;
		}else if(random==2){
			ranges->lowerBound[query][DATE_COLUMN]=730;
			ranges->upperBound[query][DATE_COLUMN]=1095-1;
		}else if(random==3){
			ranges->lowerBound[query][DATE_COLUMN]=1095;
			ranges->upperBound[query][DATE_COLUMN]=1460-1;
		}else if(random==4){
			ranges->lowerBound[query][DATE_COLUMN]=1460;
			ranges->upperBound[query][DATE_COLUMN]=1826-1;
		}else{ // == 5
			ranges->lowerBound[query][DATE_COLUMN]=1826;
			ranges->upperBound[query][DATE_COLUMN]=2191-1;
		}
		//*****************END DATE

		//*****************BEGIN Discount
		/* Possible values for discount interval
			[1, 3];
			[2, 4];
			[3, 5];
			[4, 6];
			[5, 7];
			[6, 8];
			[7, 9];
			[8, 10];
		*/
		random = (rand()%8)+1;
		ranges->lowerBound[query][DISCOUNT_COLUMN]=random;
		ranges->upperBound[query][DISCOUNT_COLUMN]=random+2;
		//*****************END Discount

		//*****************BEGIN Quantity
		/* Possible values for discount interval
			[0, 24];
			[0, 25];
		*/
		random = rand()%2;
		if(random==0){
			ranges->upperBound[query][QUANTITY_COLUMN] = 24;
		}else{
			ranges->upperBound[query][QUANTITY_COLUMN] = 25;
		}
		//*****************END Quantity


		//*****************BEGIN Linestatus
		/* Possible values for discount interval
			[0, 24];
			[0, 25];
		*/
		random = rand()%2;
		
		ranges->lowerBound[query][LINESTATUS]=random;
		ranges->upperBound[query][LINESTATUS]=random;
		//*****************END Quantityy


		//*****************BEGIN Linestatus
		/* Possible values for discount interval
			[0, 24];
			[0, 25];
		*/
		random = rand()%2;
		
		ranges->lowerBound[query][RETURNFLAG]=random;
		ranges->upperBound[query][RETURNFLAG]=random+1;
		//*****************END Quantity
         }
// Update Wildcards!
	for(uint32_t dim=0;dim<ranges->NUM_DIM;dim++){
		ranges->columnsForSelect[dim] = 0;
	}

	
	ranges->columnsForSelect[DATE_COLUMN] = 1;
	ranges->columnsForSelect[DISCOUNT_COLUMN] = 1;
	ranges->columnsForSelect[QUANTITY_COLUMN] = 1;
	ranges->columnsForSelect[LINESTATUS] = 1;
	ranges->columnsForSelect[RETURNFLAG] = 1;
	
}



void updateVeit6(PartialMatchQueries* ranges){
	uint32_t random;

	for(uint32_t query=0;query<ranges->NUM_QUERIES;query++){

		for(uint32_t dim=0;dim<ranges->NUM_DIM;dim++){
			ranges->lowerBound[query][dim] = 0;
			ranges->upperBound[query][dim] = 0;
		}
		//*****************BEGIN DATE
		/* Possible values for start date
			0: 01-01-1992:    0
			1: 01-01-1993:  365
			2: 01-01-1994:  730
			3: 01-01-1995: 1095
			4: 01-01-1996: 1460
			5: 01-01-1997: 1826
			-: 01-01-1998: 2191
		*/
		random = rand()%6;//index start date

		if(random==0){
			ranges->lowerBound[query][DATE_COLUMN]=0;
			ranges->upperBound[query][DATE_COLUMN]=365-1;
		}else if(random==1){
			ranges->lowerBound[query][DATE_COLUMN]=365;
			ranges->upperBound[query][DATE_COLUMN]=730-1;
		}else if(random==2){
			ranges->lowerBound[query][DATE_COLUMN]=730;
			ranges->upperBound[query][DATE_COLUMN]=1095-1;
		}else if(random==3){
			ranges->lowerBound[query][DATE_COLUMN]=1095;
			ranges->upperBound[query][DATE_COLUMN]=1460-1;
		}else if(random==4){
			ranges->lowerBound[query][DATE_COLUMN]=1460;
			ranges->upperBound[query][DATE_COLUMN]=1826-1;
		}else{ // == 5
			ranges->lowerBound[query][DATE_COLUMN]=1826;
			ranges->upperBound[query][DATE_COLUMN]=2191-1;
		}
		//*****************END DATE

		//*****************BEGIN Discount
		/* Possible values for discount interval
			[1, 3];
			[2, 4];
			[3, 5];
			[4, 6];
			[5, 7];
			[6, 8];
			[7, 9];
			[8, 10];
		*/
		random = (rand()%8)+1;
		ranges->lowerBound[query][DISCOUNT_COLUMN]=random;
		ranges->upperBound[query][DISCOUNT_COLUMN]=random+2;
		//*****************END Discount

		//*****************BEGIN Quantity
		/* Possible values for discount interval
			[0, 24];
			[0, 25];
		*/
		random = rand()%2;
		if(random==0){
			ranges->upperBound[query][QUANTITY_COLUMN] = 24;
		}else{
			ranges->upperBound[query][QUANTITY_COLUMN] = 25;
		}
		//*****************END Quantity


		//*****************BEGIN Linestatus
		/* Possible values for discount interval
			[0, 24];
			[0, 25];
		*/
		random = rand()%2;
		
		ranges->lowerBound[query][LINESTATUS]=random;
		ranges->upperBound[query][LINESTATUS]=random;
		//*****************END Quantityy


		//*****************BEGIN RETURNFLAG
		/* Possible values for discount interval
			[0, 24];
			[0, 25];
		*/
		random = rand()%2;
		
		ranges->lowerBound[query][RETURNFLAG]=random;
		ranges->upperBound[query][RETURNFLAG]=random+1;
		//*****************END Quantity



		//*****************BEGIN SHIPINSTR
		/* Possible values for discount interval
			[0, 24];
			[0, 25];
		*/
		random = rand()%3;
		
		ranges->lowerBound[query][SHIPINSTR]=random;
		ranges->upperBound[query][SHIPINSTR]=random+1;
		//*****************END Quantity
         }
// Update Wildcards!
	for(uint32_t dim=0;dim<ranges->NUM_DIM;dim++){
		ranges->columnsForSelect[dim] = 0;
	}

	
	ranges->columnsForSelect[DATE_COLUMN] = 1;
	ranges->columnsForSelect[DISCOUNT_COLUMN] = 1;
	ranges->columnsForSelect[QUANTITY_COLUMN] = 1;
	ranges->columnsForSelect[LINESTATUS] = 1;
	ranges->columnsForSelect[RETURNFLAG] = 1;
	ranges->columnsForSelect[SHIPINSTR] = 1;
	
}

void get(bool* columsForSelect, uint32_t NUM_DIM){

}


void get(bool* columsForSelect, uint32_t NUM_DIM, uint32_t num){
    for(uint32_t dim=0;dim<NUM_DIM;dim++){
        if(dim < num)
            columsForSelect[dim] = 1;
        else
            columsForSelect[dim] = 0;
    }
    
}
#endif // TPC_H_H
