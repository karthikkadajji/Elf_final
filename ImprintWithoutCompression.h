#ifndef IMPRINTWITHOUTCOMPRESSION_H
#define IMPRINTWITHOUTCOMPRESSION_H

#include "Imprint.h"
#include "Util.h"
#include <algorithm>
#include <string.h>

struct ColumnImprint {
	uint32_t *bins;		/* pointer into imprints heap (bins borders)  */
	uint64_t *imps;		/* pointer into imprints heap (bit vectors)   */
};

class ImprintIndexWithoutCompression : Index
{
	private:
		vector<uint32_t*> *cols;
		uint8_t* result;
		vector<ColumnImprint*> columnImprints;
		const uint32_t NUM_CACHE_LINES;
		const uint32_t NUM_DIM;

		void createColumnImprint(uint32_t* column, uint32_t* sample, uint32_t curDim){
			ColumnImprint* imprints = columnImprints[curDim];

		#ifdef NO_SIMD_OPERATIONS
			imprints->bins = (uint32_t*) malloc(64*sizeof(uint32_t));
			imprints->imps = (uint64_t*) malloc(this->NUM_CACHE_LINES*sizeof(uint64_t));//XXX
		#else
			void* temp;
			if(posix_memalign(&temp,16,200*sizeof(uint32_t))!=0){
				cout << "memalign failed!"<<endl;
			}
			imprints->bins=(uint32_t*)temp;

			if(posix_memalign(&temp,16,this->NUM_CACHE_LINES*sizeof(uint64_t))!=0){
				cout << "memalign failed!"<<endl;
			}
			imprints->imps=(uint64_t*)temp;
		#endif

			//*************** Begin Fill Histogram *********************************//
			uint32_t k;
			double y, ystep = (double) STORE->NUM_POINTS / (64 - 1);
			for (k = 0, y = 0; (uint32_t) y < STORE->NUM_POINTS; y += ystep, k++){
				imprints->bins[k] = sample[(uint32_t) y];
			}
			if (k == 64 - 1){  //there is one left
				imprints->bins[k] = sample[STORE->NUM_POINTS - 1];
			}
			//*************** End Fill Histogram *********************************//


			//*************** Begin Create Imprints ******************************
			uint64_t mask = 0;
			uint32_t newCacheLine = (IMPS_PAGE/sizeof(int))-1;
			uint32_t impCount = 0;
			uint32_t bit_to_set = 0;

			for (uint32_t tid = 0; tid < STORE->NUM_POINTS; tid++) {
				if(!(tid&newCacheLine) && tid>0){//finished this cache line
					imprints->imps[impCount++] = mask;
					mask = 0;
				}

				bit_to_set=GETBIN64(column[tid], imprints->bins);
				mask = IMPSsetBit(64,mask,bit_to_set);
			}

			//The last chase line may not be full and is thus not added to the imps
			imprints->imps[impCount++] = mask;
			//assert(impCount==this->NUM_CACHE_LINES);
			//*************** End Create Imprints ******************************//
		}

		inline uint64_t make_mask(const ColumnImprint* imprints, const uint32_t lower, const uint32_t upper, uint64_t* tempInnermask){
			uint32_t lowerBitToSet=0, upperBitToSet=0;
			lowerBitToSet=GETBIN64( lower, imprints->bins);
			upperBitToSet=GETBIN64( upper, imprints->bins);
			uint64_t mask = (((((uint64_t) 1 << upperBitToSet) - 1) << 1) | 1) - (((uint64_t) 1 << lowerBitToSet) - 1);
			*tempInnermask = mask;
			if(lower != STORE->DIM_MIN)//und nicht imprints->bins[lowerBitToSet] == lower
				*tempInnermask = IMPSunsetBit(64, *tempInnermask, lowerBitToSet);
			if (upper != STORE->DIM_MAX)
				*tempInnermask = IMPSunsetBit(64, *tempInnermask, upperBitToSet);
			return mask;
		}

		uint8_t* range_select_column(const uint32_t curDim, const uint32_t lower, const uint32_t upper){
			uint8_t* result = this->result;
			uint32_t curCacheLine = 0;
			const ColumnImprint* imprints = this->columnImprints[curDim];
			const uint64_t* IMPRINT_VECTORS = imprints->imps;
			uint64_t tempInnermask = 0;
			const uint64_t mask = make_mask(imprints, lower, upper, &tempInnermask);
			const uint64_t innermask = ~tempInnermask;//we compute the complement only once, and this way i can use a const value
			uint64_t curImprint;

			for(curCacheLine = 0;curCacheLine<NUM_CACHE_LINES;curCacheLine++){
				curImprint = IMPRINT_VECTORS[curCacheLine];
				uint32_t tid = curCacheLine*16;

				if(curImprint&mask){//hit outer mask
					if ((curImprint&innermask)){//need to filter false positives
						checkTids_AND((*cols)[curDim],tid, result, lower, upper);
					}
				}else{
//                    count++;
					result[tid/8]=0;
					result[(tid/8)+1]=0;
				}
			}
			return result;
		}

		inline uint64_t make_mask_exact(uint32_t compareValue){
			uint32_t bitToSet = GETBIN64(compareValue, columnImprints[0]->bins);
			uint64_t mask=0;
			mask=IMPSsetBit(64,mask,bitToSet);
			return mask;
		}

		inline bool isEqualCStore(const int TID, uint32_t* query, const uint32_t LENGTH){
			uint32_t value;
			uint32_t* column;

			for(uint32_t dim=FIRST_DIM;dim<LENGTH;dim++){
				column = (*cols)[dim];
				value = column[TID];
				if(value!=query[dim]){
					return false;
				}
			}
			return true;
		}

//		int count;
	public:
		ImprintIndexWithoutCompression(Store* s) : Index(s), NUM_CACHE_LINES((s->NUM_POINTS+15)/16), NUM_DIM(s->NUM_DIM){
			for (uint32_t dim = 0; dim < s->NUM_DIM; dim++) {
				ColumnImprint* dummy = new ColumnImprint();
				columnImprints.push_back(dummy);
			}
			cols = new vector<uint32_t*>(NUM_DIM);
			result = new uint8_t[(s->NUM_POINTS+7)/8];
//			count = 0;
		}

		void buildIndex(){
			uint32_t* col;
			//uint8_t* res;
			vector<uint32_t>*sample = new vector<uint32_t>(STORE->NUM_POINTS);

			for(uint32_t dim = 0; dim < STORE->NUM_DIM; dim++){
			#ifndef NO_SIMD_OPERATIONS
				//allocate columns
				void* temp;
				if(posix_memalign(&temp, IMPS_PAGE, (STORE->NUM_POINTS+16)*sizeof(uint32_t))!=0){

                                        cout << "MemAlign failed! "<< endl;
                                        exit(-1);
                                }
				col =(uint32_t*)temp;
				//allocate result vectors, I want to allocate always a whole SIMD lane
				if(posix_memalign(&temp, IMPS_PAGE, ((STORE->NUM_POINTS+15)/16)*2)!=0){
                                        cout << "MemAlign failed! "<< endl;
                                        exit(-1);
                                };
				//res= (uint8_t*)temp;
			#else
				col = new uint32_t[STORE->NUM_POINTS+16];//allocate columns
				uint32_t size = (STORE->NUM_POINTS+7)/8;
				res = new uint8_t[size];//allocate result vectors
				res[size-1] = 0;
			#endif
                uint32_t dummy;
				for (uint32_t point = 0; point < STORE->NUM_POINTS; point++) {
					dummy = STORE->getPoint(point)[dim];
					col[point]=dummy;
					(*sample)[point]=dummy;
				}
                std::sort(sample->begin(), sample->end());
				(*cols)[dim]=col;
				createColumnImprint(col, &((*sample)[0]), dim);
			}
			delete sample;
		}

		uint32_t exactMatch(uint32_t* query){
			uint64_t mask;
			mask = make_mask_exact(query[0]);
			uint64_t curImprint;
			const uint64_t* IMPRINT_VECTORS = this->columnImprints[0]->imps;
			uint32_t tid;

			for(uint32_t curCacheLine = 0;curCacheLine<NUM_CACHE_LINES;curCacheLine++){
				curImprint = IMPRINT_VECTORS[curCacheLine];

				if(curImprint&mask){//hit mask
					for(tid = curCacheLine*16;tid<(curCacheLine+1)*16;tid++){
						if(isEqualCStore(tid, query, NUM_DIM)){
							return tid;
						}
					}
                }else{
//                    count++;
                }
			}
			return NOT_FOUND;
		}
    
    
    vector<uint32_t>* partialMatch(uint32_t* lowerBoundQuery, uint32_t* upperBoundQuery, bool* columnsForSelect){
        vector<uint32_t>* resultTIDs = new vector<uint32_t>();;
        memset(this->result,255,(STORE->NUM_POINTS+7)/8);
        
        for (uint32_t dim = 0; dim < STORE->NUM_DIM; dim++) {
            if(columnsForSelect[dim]){
                range_select_column(dim, lowerBoundQuery[dim], upperBoundQuery[dim]);
            }
        }
        returnTids(resultTIDs, this->result,STORE->NUM_POINTS);
        
        
        return resultTIDs;
    }

		vector<uint32_t>* windowQuery(uint32_t* lowerBoundQuery, uint32_t* upperBoundQuery){
			vector<uint32_t>* resultTIDs = new vector<uint32_t>();
			memset(this->result,255,(STORE->NUM_POINTS+7)/8);

			for (uint32_t dim = 0; dim < NUM_DIM; dim++) {
				range_select_column(dim, lowerBoundQuery[dim], upperBoundQuery[dim]);
			}
			returnTids(resultTIDs, this->result,STORE->NUM_POINTS);

			return resultTIDs;
		}



		~ImprintIndexWithoutCompression(){
			for(uint32_t dim = 0; dim < NUM_DIM; dim++){
				free ((*cols)[dim]);
//				free (result);
				free(columnImprints[dim]->bins);
				free(columnImprints[dim]->imps);
				delete columnImprints[dim];
			}
			delete cols;
			delete[] result;
		}
};

#endif // IMPRINTWITHOUTCOMPRESSION_H
