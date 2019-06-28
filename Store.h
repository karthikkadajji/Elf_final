#ifndef STORE_H
#define STORE_H


#include <iostream>
#include <stdint.h>

using namespace std;

class Store
{
	public:
		uint32_t NUM_DIM;
		uint32_t NUM_POINTS;
		uint32_t DIM_MIN;
		uint32_t DIM_MAX;
		uint32_t **STORE;

		Store(uint32_t dim, uint32_t size, uint32_t dimMin, uint32_t dimMax) : NUM_DIM(dim), NUM_POINTS(size), DIM_MIN(dimMin), DIM_MAX(dimMax){
			STORE = new uint32_t*[NUM_POINTS];
			/*for(uint32_t i=0;i<NUM_POINT;i++){
				STORE[i] = new uint32_t[NUM_DIM];
			}*/

			currentTID = -1;
		}

		~Store(){
			for (uint32_t i = 0; i < NUM_POINTS; ++i){
				delete [] STORE[i];
			}
			delete [] STORE;
		}

		uint32_t insert(uint32_t* toInsert){
			uint32_t TID = getNextTID();
			//TODO Check length point
			//TODO check max NUM Points
			STORE[TID] = toInsert;
			return TID;
		}

		inline uint32_t* getPoint(uint32_t TID){
			return STORE[TID];
		}

		uint32_t size() {
			return NUM_POINTS;
		}

		void bulkInsertWithoutCheck(uint32_t** DATA){
//			cout << "bulkInsertWithoutCheck(): start" << endl;
			for(currentTID=0;currentTID<NUM_POINTS;currentTID++){
				STORE[currentTID]=DATA[currentTID];
			}
//			cout << "bulkInsertWithoutCheck(): stop" << endl;
		}

		void out(){
			cout << "Dim "<< NUM_DIM << " at " << NUM_POINTS << endl;

			for(uint32_t point = 0;point<NUM_POINTS; point++){
				cout << point << ":\t";
				for(uint32_t dim=0;dim<NUM_DIM;dim++){
					cout << STORE[point][dim] << " ";
				}
				cout << endl;
			}
		}

	private:
		uint32_t currentTID;

		uint32_t getNextTID() {
			return currentTID++;
		}

};

#endif // STORE_H
