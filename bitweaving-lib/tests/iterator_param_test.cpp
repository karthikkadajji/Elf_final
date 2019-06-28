// This file copyright (c) 2011-2013, the BitWeaving authors.
// All rights reserved.
// See file LICENSE for details.
#include <string>

#include "bitweaving/bitvector.h"
#include "bitweaving/column.h"
#include "bitweaving/iterator.h"
#include "bitweaving/table.h"
#include "bitweaving/types.h"
#include "gtest/gtest.h"
#include "src/bitvector_block.h"
#include "src/macros.h"

namespace bitweaving {

class IteratorParamTest : public ::testing::TestWithParam<size_t> {
 protected:
  virtual void SetUp() {
    Options options = bitweaving::Options();
    options.in_memory = true;
    table_ = new Table(std::string(), options);

    code_size_ = GetParam();

    Column *column;

    table_->AddColumn("bwh", bitweaving::kBitWeavingH, code_size_);
    column = table_->GetColumn("bwh");

    for (size_t i = 0; i < NUM_TEST_CODES; i++) {
      data_[i] = i % (1 << code_size_);
    }
    column->Bulkload(data_, NUM_TEST_CODES);

    table_->AddColumn("bwv", bitweaving::kBitWeavingV, code_size_);
    column = table_->GetColumn("bwv");

    for (size_t i = 0; i < NUM_TEST_CODES; i++) {
      data_[i] = i % (1 << code_size_);
    }
    column->Bulkload(data_, NUM_TEST_CODES);
  }

  virtual void TearDown() {
    delete table_;
  }

  static const size_t NUM_TEST_CODES = 1000;
  size_t code_size_;
  Code data_[NUM_TEST_CODES];
  Table *table_;
};

INSTANTIATE_TEST_CASE_P(IteratorCodeSize, IteratorParamTest,
                        ::testing::Values(1U, 6U, 7U, 8U, 13U));

TEST_P(IteratorParamTest, BwHIteratorGetCode) {
  Code code, id;
  size_t count;
  size_t step;
  BitVector *bitvector = new BitVector(*table_);
  Iterator *iter = new Iterator(*bitvector);
  Column *column = table_->GetColumn("bwh");

  id = 0;
  count = 0;
  iter->Begin();
  while (iter->Next()) {
    // Get the next matching value
    iter->GetCode(*column, &code);
    EXPECT_EQ(id % (1 << code_size_), code);
    id++;
    count++;
  }
  EXPECT_TRUE(count == NUM_TEST_CODES);

  // Test the step = 10
  step = 10;
  bitvector->SetZeros();
  for (size_t i = 0; i < NUM_TEST_CODES; i += step) {
    bitvector->SetBit(i, 1);
  }

  id = 0;
  count = 0;
  iter->Begin();
  while (iter->Next()) {
    // Get the next matching value
    iter->GetCode(*column, &code);
    EXPECT_EQ(id % (1 << code_size_), code);
    id += step;
    count++;
  }
  EXPECT_TRUE(count == CEIL(NUM_TEST_CODES, step));

  // Test the step = 97
  step = 97;
  bitvector->SetZeros();
  for (size_t i = 0; i < NUM_TEST_CODES; i += step) {
    bitvector->SetBit(i, 1);
  }

  id = 0;
  count = 0;
  iter->Begin();
  while (iter->Next()) {
    // Get the next matching value
    iter->GetCode(*column, &code);
    EXPECT_EQ(id % (1 << code_size_), code);
    id += step;
    count++;
  }
  EXPECT_TRUE(count == CEIL(NUM_TEST_CODES, step));

  // Test the step = NUM_CODES_PER_BLOCK + 10
  step = kNumCodesPerBlock + 10;
  bitvector->SetZeros();
  for (size_t i = 0; i < NUM_TEST_CODES; i += step) {
    bitvector->SetBit(i, 1);
  }

  id = 0;
  count = 0;
  iter->Begin();
  while (iter->Next()) {
    // Get the next matching value
    iter->GetCode(*column, &code);
    EXPECT_EQ(id % (1 << code_size_), code);
    id += step;
    count++;
  }
  EXPECT_TRUE(count == CEIL(NUM_TEST_CODES, step));

  delete iter;
  delete bitvector;
}

TEST_P(IteratorParamTest, BwHIteratorSetCode) {
  Code code, id;
  size_t count;
  size_t step;
  BitVector *bitvector = new BitVector(*table_);
  Column *column = table_->GetColumn("bwh");
  Iterator *iter = new Iterator(*bitvector);

  // Test the step = 97
  step = 97;
  bitvector->SetZeros();
  for (size_t i = 0; i < NUM_TEST_CODES; i += step) {
    bitvector->SetBit(i, 1);
  }

  id = 0;
  count = 0;
  iter->Begin();
  while (iter->Next()) {
    // Get the next matching value
    iter->SetCode(*column, (id + 1) % (1 << code_size_));
    id += step;
    count++;
  }
  EXPECT_TRUE(count == CEIL(NUM_TEST_CODES, step));

  id = 0;
  count = 0;
  iter->Begin();
  while (iter->Next()) {
    // Get the next matching value
    iter->GetCode(*column, &code);
    EXPECT_EQ((id + 1) % (1 << code_size_), code);
    id += step;
    count++;
  }
  EXPECT_TRUE(count == CEIL(NUM_TEST_CODES, step));

  bitvector->SetOnes();

  id = 0;
  count = 0;
  iter->Begin();
  while (iter->Next()) {
    // Get the next matching value
    iter->SetCode(*column, (id + 1) % (1 << code_size_));
    id++;
    count++;
  }
  EXPECT_TRUE(count == NUM_TEST_CODES);

  id = 0;
  count = 0;
  iter->Begin();
  while (iter->Next()) {
    // Get the next matching value
    iter->GetCode(*column, &code);
    EXPECT_EQ((id + 1) % (1 << code_size_), code);
    id++;
    count++;
  }
  EXPECT_TRUE(count == NUM_TEST_CODES);

  delete iter;
  delete bitvector;
}

TEST_P(IteratorParamTest, BwVIteratorGetCode) {
  Code code, id;
  size_t count;
  size_t step;
  Column *column = table_->GetColumn("bwv");
  BitVector *bitvector = new BitVector(*table_);
  Iterator *iter = new Iterator(*bitvector);

  id = 0;
  count = 0;
  iter->Begin();
  while (iter->Next()) {
    // Get the next matching value
    iter->GetCode(*column, &code);
    EXPECT_EQ(id % (1 << code_size_), code);
    id++;
    count++;
  }
  EXPECT_TRUE(count == NUM_TEST_CODES);

  // Test the step = 10
  step = 10;
  bitvector->SetZeros();
  for (size_t i = 0; i < NUM_TEST_CODES; i += step) {
    bitvector->SetBit(i, 1);
  }

  id = 0;
  count = 0;
  iter->Begin();
  while (iter->Next()) {
    // Get the next matching value
    iter->GetCode(*column, &code);
    EXPECT_EQ(id % (1 << code_size_), code);
    id += step;
    count++;
  }
  EXPECT_TRUE(count == CEIL(NUM_TEST_CODES, step));

  // Test the step = 97
  step = 97;
  bitvector->SetZeros();
  for (size_t i = 0; i < NUM_TEST_CODES; i += step) {
    bitvector->SetBit(i, 1);
  }

  id = 0;
  count = 0;
  iter->Begin();
  while (iter->Next()) {
    // Get the next matching value
    iter->GetCode(*column, &code);
    EXPECT_EQ(id % (1 << code_size_), code);
    id += step;
    count++;
  }
  EXPECT_TRUE(count == CEIL(NUM_TEST_CODES, step));

  // Test the step = NUM_CODES_PER_BLOCK + 10
  step = kNumCodesPerBlock + 10;
  bitvector->SetZeros();
  for (size_t i = 0; i < NUM_TEST_CODES; i += step) {
    bitvector->SetBit(i, 1);
  }

  id = 0;
  count = 0;
  iter->Begin();
  while (iter->Next()) {
    // Get the next matching value
    iter->GetCode(*column, &code);
    EXPECT_EQ(id % (1 << code_size_), code);
    id += step;
    count++;
  }
  EXPECT_TRUE(count == CEIL(NUM_TEST_CODES, step));

  delete iter;
  delete bitvector;
}

TEST_P(IteratorParamTest, BwVIteratorSetCode) {
  Code code, id;
  size_t count;
  size_t step;
  Column *column = table_->GetColumn("bwv");
  BitVector* bitvector = new BitVector(*table_);
  Iterator* iter = new Iterator(*bitvector);

  // Test the step = 97
  step = 97;
  bitvector->SetZeros();
  for (size_t i = 0; i < NUM_TEST_CODES; i += step) {
    bitvector->SetBit(i, 1);
  }

  id = 0;
  count = 0;
  iter->Begin();
  while (iter->Next()) {
    // Get the next matching value
    iter->SetCode(*column, (id + 1) % (1 << code_size_));
    id += step;
    count++;
  }
  EXPECT_TRUE(count == CEIL(NUM_TEST_CODES, step));

  id = 0;
  count = 0;
  iter->Begin();
  while (iter->Next()) {
    // Get the next matching value
    iter->GetCode(*column, &code);
    EXPECT_EQ((id + 1) % (1 << code_size_), code);
    id += step;
    count++;
  }
  EXPECT_TRUE(count == CEIL(NUM_TEST_CODES, step));

  bitvector->SetOnes();

  id = 0;
  count = 0;
  iter->Begin();
  while (iter->Next()) {
    // Get the next matching value
    iter->SetCode(*column, (id + 1) % (1 << code_size_));
    id++;
    count++;
  }
  EXPECT_TRUE(count == NUM_TEST_CODES);

  id = 0;
  count = 0;
  iter->Begin();
  while (iter->Next()) {
    // Get the next matching value
    iter->GetCode(*column, &code);
    EXPECT_EQ((id + 1) % (1 << code_size_), code);
    id++;
    count++;
  }
  EXPECT_TRUE(count == NUM_TEST_CODES);

  delete iter;
  delete bitvector;
}
}
