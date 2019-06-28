// This file copyright (c) 2011-2013, the BitWeaving authors.
// All rights reserved.
// See file LICENSE for details.

#include "bitweaving/table.h"
#include "bitweaving/types.h"
#include "bitweaving/bitvector.h"
#include "gtest/gtest.h"
#include "src/bitvector_block.h"
#include "src/bitvector_iterator.h"

namespace bitweaving {

class BitVectorIteratorTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    ids[0] = 1;
    ids[1] = 2;
    ids[2] = 10;
    ids[3] = 63;
    ids[4] = 128;
    ids[5] = 191;
    ids[6] = 256;
    ids[7] = 400;

    table_ = CreateMockedTable(NUM_TEST_BITS);
    bitvector_ = new BitVector(*table_);
    bitvector_->SetZeros();
    for (size_t i = 0; i < NUM_SET_BITS; i++) {
      bitvector_->SetBit(ids[i], 1);
    }
  }

  virtual void TearDown() {
    delete bitvector_;
    delete table_;
  }

  Table* CreateMockedTable(size_t size) {
    Options options;
    options.in_memory = true;
    Table *table = new Table("", options);
    table->Resize(size);
    return table;
  }

  Table *table_;
  BitVector *bitvector_;
  static const size_t NUM_SET_BITS = 8;
  static const size_t NUM_TEST_BITS = 500;
  TupleId ids[NUM_SET_BITS];
};

TEST_F(BitVectorIteratorTest, SpecialBitVector) {
  BitVectorIterator *iter;
  size_t count;

  // Iterator on an all-zero bitvector
  bitvector_->SetZeros();
  iter = new BitVectorIterator(*bitvector_);

  count = 0;
  while (iter->Next()) {
    count++;
  }
  EXPECT_EQ(0u, count);

  // Iterator on an all-one bitvector
  bitvector_->SetOnes();
  iter->Begin();
  count = 0;
  while (iter->Next()) {
    count++;
  }
  EXPECT_TRUE(NUM_TEST_BITS == count);

  delete iter;
}

TEST_F(BitVectorIteratorTest, Basic) {
  BitVectorIterator *iter = new BitVectorIterator(*bitvector_);
  size_t count = 0;
  while (iter->Next()) {
    // Get the next matching value
    TupleId id = iter->GetPosition();
    EXPECT_EQ(id, ids[count]);
    count++;
  }
  EXPECT_TRUE(NUM_SET_BITS == count);
  delete iter;
}

}  // namespace bitweaving
