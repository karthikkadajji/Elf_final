#ifndef MYKDTREE_Small_H
#define MYKDTREE_Small_H

#include "Util.h"
#include "Store.h"

class MyKDTreeSmall : public Index
{
	public:
		uint32_t NUM_DIM;
		uint32_t NUM_POINTS;

		static const uint32_t DIM_VALUE_OFFSET 		= 0;
		static const uint32_t POINTER_LEFT_OFFSET 	= DIM_VALUE_OFFSET+1;//1
		static const uint32_t POINTER_RIGHT_OFFSET 	= POINTER_LEFT_OFFSET+1;//2
		static const uint32_t NODE_SIZE 				= POINTER_RIGHT_OFFSET+1;//3

		static const uint32_t HAS_NO_CHILD 	= 0;
		static const uint32_t ROOT_OFFSET 	= 0;
		static const uint32_t START_DIM 		= 0;

		uint32_t nextNodeOffset;//should be private

		MyKDTreeSmall(Store* s) : Index(s), NUM_DIM(s->NUM_DIM), NUM_POINTS(s->NUM_POINTS){
			TREE = new uint32_t[NUM_POINTS*NODE_SIZE];
		}

		void buildIndex() {
			uint32_t* point = this->STORE->getPoint(FIRST_TID);
			nextNodeOffset = ROOT_OFFSET;

			// create ROOT
			newKDNode(STORE->getPoint(FIRST_TID),START_DIM);

			for(uint32_t TID=FIRST_TID+1;TID<NUM_POINTS;TID++){
				point = STORE->getPoint(TID);
				insert(point);
			}
		}

		uint32_t exactMatch(uint32_t* query) {
			uint32_t node = ROOT_OFFSET;
			uint32_t dimForCompare = 0;
			uint32_t* point;
			uint32_t tid;
			uint32_t value;

			do{
				value = getDimValue(node);
				if(query[dimForCompare]==value){//Match in this dimension. So, point possibly found.
					tid = getTID(node);
					point = STORE->getPoint(tid);
					if(isEqual(query, point, NUM_DIM)){
						return tid;
					}
				}
				node = (query[dimForCompare]<=value) ? getLeftChildOffset(node) : getRightChildOffset(node);
				dimForCompare = (dimForCompare!=NUM_DIM-1) ? dimForCompare+1 : 0;
			}while(node!=HAS_NO_CHILD);

			return NOT_FOUND;
		}

		vector<uint32_t>* windowQuery(uint32_t* lowerBoundQuery, uint32_t* upperBoundQuery){
			vector<uint32_t>* resultTIDs = new vector<uint32_t>();;
			windowQuery(ROOT_OFFSET, lowerBoundQuery, upperBoundQuery, resultTIDs, FIRST_DIM);
//			std::sort (resultTIDs->begin(), resultTIDs->end());
			return resultTIDs;
		}

		vector<uint32_t>* partialMatch(uint32_t* lowerBoundQuery, uint32_t* upperBoundQuery,  bool* columnsForSelect){
			vector<uint32_t>* resultTIDs = new vector<uint32_t>();
			partialMatch(ROOT_OFFSET, lowerBoundQuery, upperBoundQuery, resultTIDs, FIRST_DIM,  columnsForSelect);
//			std::sort (resultTIDs->begin(), resultTIDs->end());
			return resultTIDs;
		}

		~MyKDTreeSmall(){
			delete TREE;;
		}

	private:
		/** The actual tree encoded as 32-bit signed integer values.*/
		uint32_t* TREE;

		/**
		 * Pseudo constructor.
		 * @returns Offset where node was created.
		 */
		uint32_t newKDNode(uint32_t* TO_INSERT, uint32_t DIM_FOR_COMPARE) {
			uint32_t MY_OFFSET = nextNodeOffset;
			nextNodeOffset += NODE_SIZE;

			setDimValue(MY_OFFSET, TO_INSERT[DIM_FOR_COMPARE]);
			setLeftChildOffset(MY_OFFSET, HAS_NO_CHILD);
			setRightChildOffset(MY_OFFSET, HAS_NO_CHILD);

			return MY_OFFSET;
		}
		inline uint32_t getDimValue(uint32_t OFFSET){
			return TREE[OFFSET+DIM_VALUE_OFFSET];
		}
		inline uint32_t getTID(uint32_t OFFSET){
			return OFFSET/NODE_SIZE;
		}
		inline uint32_t getLeftChildOffset(uint32_t OFFSET){
			return TREE[OFFSET+POINTER_LEFT_OFFSET];
		}
		inline uint32_t getRightChildOffset(uint32_t OFFSET){
			return TREE[OFFSET+POINTER_RIGHT_OFFSET];
		}
		inline void setDimValue(uint32_t OFFSET, uint32_t VALUE){
			TREE[OFFSET+DIM_VALUE_OFFSET] = VALUE;
		}
		inline void setLeftChildOffset(uint32_t OFFSET, uint32_t LEFT_CHILD){
			TREE[OFFSET+POINTER_LEFT_OFFSET] = LEFT_CHILD;
		}
		inline void setRightChildOffset(uint32_t OFFSET, uint32_t RIGHT_CHILD){
			TREE[OFFSET+POINTER_RIGHT_OFFSET] = RIGHT_CHILD;
		}

		void insert(uint32_t* TO_INSERT) {
			uint32_t parent = ROOT_OFFSET;
			uint32_t nextNode = ROOT_OFFSET;

			uint32_t dimForCompare = -1;//increment makes 0 for first loop

			do{
				parent = nextNode;
				dimForCompare = (dimForCompare!=NUM_DIM-1) ? dimForCompare+1 : 0;
				nextNode = (TO_INSERT[dimForCompare]<=getDimValue(parent)) ? getLeftChildOffset(parent) : getRightChildOffset(parent);
			}while(nextNode!=HAS_NO_CHILD);

			uint32_t OFFSET = newKDNode(TO_INSERT, (dimForCompare!=NUM_DIM-1) ? dimForCompare+1 : 0);
			if(TO_INSERT[dimForCompare]<= getDimValue(parent)){
				setLeftChildOffset(parent, OFFSET);
			}else{
				setRightChildOffset(parent, OFFSET);
			}
		}

		void windowQuery(const uint32_t NODE, const uint32_t* lowerBoundQuery, const uint32_t* upperBoundQuery
					, vector<uint32_t>* resultTIDS, const uint32_t dimForCompare) {

			//XXX erst mit dem compValue, damit ich den Punkt nicht holen muss?
			uint32_t TID = getTID(NODE);
			uint32_t* point = STORE->getPoint(TID);
			if(isIn(point, lowerBoundQuery, upperBoundQuery, NUM_DIM))
				resultTIDS->push_back(TID);

			uint32_t nextDimForCompare = (dimForCompare!=NUM_DIM-1) ? dimForCompare+1 : 0;
			const uint32_t compValue = getDimValue(NODE);

			if(lowerBoundQuery[dimForCompare]<=compValue){//dim lowerBoundQuery links oder gleich von comp value
				uint32_t left = getLeftChildOffset(NODE);
				if(left != HAS_NO_CHILD)
					windowQuery(left, lowerBoundQuery, upperBoundQuery, resultTIDS, nextDimForCompare);
			}
			if(upperBoundQuery[dimForCompare]>=compValue){//right branch
				uint32_t right = getRightChildOffset(NODE);
				if(right != HAS_NO_CHILD)
					windowQuery(right,lowerBoundQuery, upperBoundQuery, resultTIDS, nextDimForCompare);
			}
		}

		void partialMatch(const uint32_t NODE, const uint32_t* lowerBoundQuery, const uint32_t* upperBoundQuery
					, vector<uint32_t>* resultTIDS, const uint32_t dimForCompare, const bool* columnsForSelect) {

			//XXX erst mit dem compValue, damit ich den Punkt nicht holen muss?
			uint32_t TID = getTID(NODE);
			uint32_t* point = STORE->getPoint(TID);
			if(isPartiallyIn(point, lowerBoundQuery, upperBoundQuery, columnsForSelect, NUM_DIM))
				resultTIDS->push_back(TID);

			uint32_t nextDimForCompare = (dimForCompare!=NUM_DIM-1) ? dimForCompare+1 : 0;
			const uint32_t compValue = getDimValue(NODE);

			if(lowerBoundQuery[dimForCompare]<=compValue || columnsForSelect[dimForCompare]==false){//dim lowerBoundQuery links oder gleich von comp value
				uint32_t left = getLeftChildOffset(NODE);
				if(left != HAS_NO_CHILD)
					partialMatch(left, lowerBoundQuery, upperBoundQuery, resultTIDS, nextDimForCompare, columnsForSelect);
			}
			if(upperBoundQuery[dimForCompare]>=compValue || columnsForSelect[dimForCompare]==false){//right branch
				uint32_t right = getRightChildOffset(NODE);
				if(right != HAS_NO_CHILD)
					partialMatch(right,lowerBoundQuery, upperBoundQuery, resultTIDS, nextDimForCompare, columnsForSelect);
			}
		}

};

#endif // MYKDTREE_H

