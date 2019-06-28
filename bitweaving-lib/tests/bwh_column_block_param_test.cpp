// This file copyright (c) 2011-2013, the BitWeaving authors.
// All rights reserved.
// See file LICENSE for details.

#include "bitweaving/types.h"
#include "gtest/gtest.h"
#include "src/column_block.h"
#include "src/bwh_column_block.h"

namespace bitweaving {

class BwHColumnBlockParamTest : public ::testing::TestWithParam<size_t> {
 protected:
  virtual void SetUp() {
    code_size_ = GetParam();
    first_block_ = CreateColumnBlock();
    second_block_ = CreateColumnBlock();

    for (size_t i = 0; i < NUM_TEST_CODES; i++) {
      first_data_[i] = rand() % (1 << code_size_);
      second_data_[i] = rand() % (1 << code_size_);
    }
    first_block_->Resize(NUM_TEST_CODES);
    second_block_->Resize(NUM_TEST_CODES);
    first_block_->Bulkload(first_data_, NUM_TEST_CODES, 0);
    second_block_->Bulkload(second_data_, NUM_TEST_CODES, 0);
  }

  virtual void TearDown() {
    delete first_block_;
    delete second_block_;
  }

  ColumnBlock * CreateColumnBlock() {
    switch (code_size_) {
      case 1:
        return new BwHColumnBlock<1>();
      case 2:
        return new BwHColumnBlock<2>();
      case 3:
        return new BwHColumnBlock<3>();
      case 4:
        return new BwHColumnBlock<4>();
      case 5:
        return new BwHColumnBlock<5>();
      case 6:
        return new BwHColumnBlock<6>();
      case 7:
        return new BwHColumnBlock<7>();
      case 8:
        return new BwHColumnBlock<8>();
      case 9:
        return new BwHColumnBlock<9>();
      case 10:
        return new BwHColumnBlock<10>();
      case 11:
        return new BwHColumnBlock<11>();
      case 12:
        return new BwHColumnBlock<12>();
      case 13:
        return new BwHColumnBlock<13>();
      case 14:
        return new BwHColumnBlock<14>();
      case 15:
        return new BwHColumnBlock<15>();
      case 16:
        return new BwHColumnBlock<16>();
      case 17:
        return new BwHColumnBlock<17>();
      case 18:
        return new BwHColumnBlock<18>();
      case 19:
        return new BwHColumnBlock<19>();
      case 20:
        return new BwHColumnBlock<20>();
      case 21:
        return new BwHColumnBlock<21>();
      case 22:
        return new BwHColumnBlock<22>();
      case 23:
        return new BwHColumnBlock<23>();
      case 24:
        return new BwHColumnBlock<24>();
      case 25:
        return new BwHColumnBlock<25>();
      case 26:
        return new BwHColumnBlock<26>();
      case 27:
        return new BwHColumnBlock<27>();
      case 28:
        return new BwHColumnBlock<28>();
      case 29:
        return new BwHColumnBlock<29>();
      case 30:
        return new BwHColumnBlock<30>();
      case 31:
        return new BwHColumnBlock<31>();
      case 32:
        return new BwHColumnBlock<32>();
    }
    return NULL;
  }

  static const size_t NUM_TEST_CODES = kNumCodesPerBlock * 3 / 4;
  size_t code_size_;
  Code first_data_[NUM_TEST_CODES];
  Code second_data_[NUM_TEST_CODES];
  ColumnBlock* first_block_;
  ColumnBlock* second_block_;
};

INSTANTIATE_TEST_CASE_P(BwHColumnBlockCodeSize, BwHColumnBlockParamTest,
                        ::testing::Values(1U, 2U, 3U, 7U, 8U));

TEST_P(BwHColumnBlockParamTest, SetGetCode) {
  Code code;
  for (size_t i = 0; i < NUM_TEST_CODES; i++) {
    first_block_->GetCode(i, &code);
    EXPECT_EQ(code, first_data_[i]);
  }
}

TEST_P(BwHColumnBlockParamTest, CompareWithLimit) {
  Code min_code = 0;
  Code max_code = (1ULL << code_size_) - 1;
  BitVectorBlock* bitvector_block = new BitVectorBlock(NUM_TEST_CODES);
  BitVectorBlock* bitvector_all_ones = new BitVectorBlock(NUM_TEST_CODES);
  BitVectorBlock* bitvector_all_zeros = new BitVectorBlock(NUM_TEST_CODES);

  bitvector_all_ones->SetOnes();
  bitvector_all_zeros->SetZeros();

  // Test Less predicate
  bitvector_block->SetZeros();
  first_block_->Scan(kLess, min_code, bitvector_block);
  EXPECT_TRUE(bitvector_block->Equal(*bitvector_all_zeros));

  // Test Greater predicate
  bitvector_block->SetZeros();
  first_block_->Scan(kGreater, max_code, bitvector_block);
  EXPECT_TRUE(bitvector_block->Equal(*bitvector_all_zeros));

  // Test LessEqual predicate
  bitvector_block->SetZeros();
  first_block_->Scan(kLessEqual, max_code, bitvector_block);
  EXPECT_TRUE(bitvector_block->Equal(*bitvector_all_ones));

  // Test GreaterEqual predicate
  bitvector_block->SetZeros();
  first_block_->Scan(kGreaterEqual, min_code, bitvector_block);
  EXPECT_TRUE(bitvector_block->Equal(*bitvector_all_ones));

  delete bitvector_block;
  delete bitvector_all_ones;
  delete bitvector_all_zeros;
}

TEST_P(BwHColumnBlockParamTest, CompareWithConstant) {
  Code code = ((1ULL << code_size_) - 1) * 2 / 3;
  BitVectorBlock* bitvector_block = new BitVectorBlock(NUM_TEST_CODES);

  // Test Equality predicate
  bitvector_block->SetZeros();
  first_block_->Scan(kEqual, code, bitvector_block);
  for (size_t i = 0; i < NUM_TEST_CODES; i++) {
    if (first_data_[i] == code) {
      EXPECT_TRUE(bitvector_block->GetBit(i));
    } else {
      EXPECT_FALSE(bitvector_block->GetBit(i));
    }
  }

  // Test Inequality predicate
  bitvector_block->SetZeros();
  first_block_->Scan(kInequal, code, bitvector_block);
  for (size_t i = 0; i < NUM_TEST_CODES; i++) {
    if (first_data_[i] != code) {
      EXPECT_TRUE(bitvector_block->GetBit(i));
    } else {
      EXPECT_FALSE(bitvector_block->GetBit(i));
    }
  }

  // Test Greater predicate
  bitvector_block->SetZeros();
  first_block_->Scan(kGreater, code, bitvector_block);
  for (size_t i = 0; i < NUM_TEST_CODES; i++) {
    if (first_data_[i] > code) {
      EXPECT_TRUE(bitvector_block->GetBit(i));
    } else {
      EXPECT_FALSE(bitvector_block->GetBit(i));
    }
  }

  // Test Less predicate
  bitvector_block->SetZeros();
  first_block_->Scan(kLess, code, bitvector_block);
  for (size_t i = 0; i < NUM_TEST_CODES; i++) {
    if (first_data_[i] < code) {
      EXPECT_TRUE(bitvector_block->GetBit(i));
    } else {
      EXPECT_FALSE(bitvector_block->GetBit(i));
    }
  }

  // Test GreaterEqual predicate
  bitvector_block->SetZeros();
  first_block_->Scan(kGreaterEqual, code, bitvector_block);
  for (size_t i = 0; i < NUM_TEST_CODES; i++) {
    if (first_data_[i] >= code) {
      EXPECT_TRUE(bitvector_block->GetBit(i));
    } else {
      EXPECT_FALSE(bitvector_block->GetBit(i));
    }
  }

  // Test LessEqual predicate
  bitvector_block->SetZeros();
  first_block_->Scan(kLessEqual, code, bitvector_block);
  for (size_t i = 0; i < NUM_TEST_CODES; i++) {
    if (first_data_[i] <= code) {
      EXPECT_TRUE(bitvector_block->GetBit(i));
    } else {
      EXPECT_FALSE(bitvector_block->GetBit(i));
    }
  }

  delete bitvector_block;
}

TEST_P(BwHColumnBlockParamTest, CompareWithColumn) {
  BitVectorBlock* bitvector_block = new BitVectorBlock(NUM_TEST_CODES);

  // Test Equality predicate
  bitvector_block->SetZeros();
  first_block_->Scan(kEqual, *second_block_, bitvector_block);
  for (size_t i = 0; i < NUM_TEST_CODES; i++) {
    if (first_data_[i] == second_data_[i]) {
      EXPECT_TRUE(bitvector_block->GetBit(i));
    } else {
      EXPECT_FALSE(bitvector_block->GetBit(i));
    }
  }

  // Test Inequality predicate
  bitvector_block->SetZeros();
  first_block_->Scan(kInequal, *second_block_, bitvector_block);
  for (size_t i = 0; i < NUM_TEST_CODES; i++) {
    if (first_data_[i] != second_data_[i]) {
      EXPECT_TRUE(bitvector_block->GetBit(i));
    } else {
      EXPECT_FALSE(bitvector_block->GetBit(i));
    }
  }

  // Test Greater predicate
  bitvector_block->SetZeros();
  first_block_->Scan(kGreater, *second_block_, bitvector_block);
  for (size_t i = 0; i < NUM_TEST_CODES; i++) {
    if (first_data_[i] > second_data_[i]) {
      EXPECT_TRUE(bitvector_block->GetBit(i));
    } else {
      EXPECT_FALSE(bitvector_block->GetBit(i));
    }
  }

  // Test Less predicate
  bitvector_block->SetZeros();
  first_block_->Scan(kLess, *second_block_, bitvector_block);
  for (size_t i = 0; i < NUM_TEST_CODES; i++) {
    if (first_data_[i] < second_data_[i]) {
      EXPECT_TRUE(bitvector_block->GetBit(i));
    } else {
      EXPECT_FALSE(bitvector_block->GetBit(i));
    }
  }

  // Test GreaterEqual predicate
  bitvector_block->SetZeros();
  first_block_->Scan(kGreaterEqual, *second_block_, bitvector_block);
  for (size_t i = 0; i < NUM_TEST_CODES; i++) {
    if (first_data_[i] >= second_data_[i]) {
      EXPECT_TRUE(bitvector_block->GetBit(i));
    } else {
      EXPECT_FALSE(bitvector_block->GetBit(i));
    }
  }

  // Test LessEqual predicate
  bitvector_block->SetZeros();
  first_block_->Scan(kLessEqual, *second_block_, bitvector_block);
  for (size_t i = 0; i < NUM_TEST_CODES; i++) {
    if (first_data_[i] <= second_data_[i]) {
      EXPECT_TRUE(bitvector_block->GetBit(i));
    } else {
      EXPECT_FALSE(bitvector_block->GetBit(i));
    }
  }

  delete bitvector_block;
}

}  // namespace bitweaving

