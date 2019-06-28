// This file copyright (c) 2011-2013, the BitWeaving authors.
// All rights reserved.
// See file LICENSE for details.

#include "bitweaving/types.h"
#include "gtest/gtest.h"
#include "src/bwv_column_block.h"
#include "src/column_block.h"
#include "src/env.h"
#include "src/file.h"
#include "src/naive_column_block.h"

namespace bitweaving {

class BwVColumnBlockTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    block_ = new BwVColumnBlock<CODE_SIZE>();
    for (size_t i = 0; i < NUM_TEST_CODES; i++) {
      data_[i] = rand() % (1 << CODE_SIZE);
    }
    block_->Resize(NUM_TEST_CODES);
    block_->Bulkload(data_, NUM_TEST_CODES, 0);
  }

  virtual void TearDown() {
    delete block_;
  }

  static const size_t NUM_TEST_CODES = kNumCodesPerBlock * 3 / 4;
  static const size_t CODE_SIZE = 4;
  Code data_[NUM_TEST_CODES];
  ColumnBlock* block_;
};

TEST_F(BwVColumnBlockTest, CheckAppendTest) {
  Code code;
  Status status;

  size_t num_exist_codes = block_->GetNumCodes();
  block_->Resize(num_exist_codes + kNumCodesPerBlock / 4);
  status = block_->Bulkload(data_, kNumCodesPerBlock / 4, num_exist_codes);
  EXPECT_TRUE(status.IsOk());
  for (size_t i = 0; i < NUM_TEST_CODES; i++) {
    block_->GetCode(i, &code);
    EXPECT_EQ(code, data_[i]);
  }
  for (size_t i = 0; i < kNumCodesPerBlock / 4; i++) {
    block_->GetCode(i + NUM_TEST_CODES, &code);
    EXPECT_EQ(code, data_[i]);
  }
}

TEST_F(BwVColumnBlockTest, CheckScanArgumentTest) {
  BitVectorBlock* bitvector = new BitVectorBlock(NUM_TEST_CODES);
  ColumnBlock* column;
  Status status;

  // Const must be within the domain
  status = block_->Scan(kEqual, 1ULL << CODE_SIZE, bitvector);
  EXPECT_FALSE(status.IsOk());
  status = block_->Scan(kEqual, 1ULL << (CODE_SIZE + 1), bitvector);
  EXPECT_FALSE(status.IsOk());

  // Code size must be same
  column = new BwVColumnBlock<CODE_SIZE + 1>();
  column->Resize(NUM_TEST_CODES);
  column->Bulkload(data_, NUM_TEST_CODES, 0);
  status = block_->Scan(kEqual, *column, bitvector);
  EXPECT_FALSE(status.IsOk());
  delete column;

  // column length must be same
  column = new BwVColumnBlock<CODE_SIZE>();
  column->Resize(NUM_TEST_CODES / 2);
  column->Bulkload(data_, NUM_TEST_CODES / 2, 0);
  status = block_->Scan(kEqual, *column, bitvector);
  EXPECT_FALSE(status.IsOk());
  delete column;

  // column type must be same
  column = new NaiveColumnBlock();
  column->Resize(NUM_TEST_CODES);
  column->Bulkload(data_, NUM_TEST_CODES, 0);
  status = block_->Scan(kEqual, *column, bitvector);
  EXPECT_FALSE(status.IsOk());
  delete column;

  // column length and bitvector length must be same
  BitVectorBlock* tmp_bitvector = new BitVectorBlock(NUM_TEST_CODES / 2);
  column = new BwVColumnBlock<CODE_SIZE>();
  column->Resize(NUM_TEST_CODES);
  column->Bulkload(data_, NUM_TEST_CODES, 0);
  status = block_->Scan(kEqual, *column, tmp_bitvector);
  EXPECT_FALSE(status.IsOk());
  status = block_->Scan(kEqual, 0, tmp_bitvector);
  EXPECT_FALSE(status.IsOk());

  delete bitvector;
  delete tmp_bitvector;
}

TEST_F(BwVColumnBlockTest, LogicalOptTest) {
  Code code = ((1ULL << CODE_SIZE) - 1) * 2 / 3;
  BitVectorBlock* bitvector1 = new BitVectorBlock(NUM_TEST_CODES);
  BitVectorBlock* bitvector2 = new BitVectorBlock(NUM_TEST_CODES);

  // The bitvector values does not depend on original values
  bitvector1->SetOnes();
  bitvector2->SetZeros();
  block_->Scan(kEqual, code, bitvector1);
  block_->Scan(kEqual, code, bitvector2);
  EXPECT_TRUE(bitvector1->Equal(*bitvector2));

  // GreaterEqual = Equal Or Greater
  block_->Scan(kEqual, code, bitvector1);
  block_->Scan(kGreater, code, bitvector1, kOr);
  block_->Scan(kGreaterEqual, code, bitvector2);
  EXPECT_TRUE(bitvector1->Equal(*bitvector2));

  // Equal And GreaterEqual = Equal
  block_->Scan(kEqual, code, bitvector1);
  block_->Scan(kGreaterEqual, code, bitvector1, kAnd);
  block_->Scan(kEqual, code, bitvector2);
  EXPECT_TRUE(bitvector1->Equal(*bitvector2));

  delete bitvector1;
  delete bitvector2;
}

TEST_F(BwVColumnBlockTest, FileOperationsTest) {
  Status status;
  SequentialWriteFile out_file;
  status = out_file.Open("./tests/data/test_bwv");
  EXPECT_TRUE(status.IsOk());

  block_->Save(out_file);

  status = out_file.Flush();
  EXPECT_TRUE(status.IsOk());
  status = out_file.Close();
  EXPECT_TRUE(status.IsOk());

  BwVColumnBlock<CODE_SIZE> *test_block = new BwVColumnBlock<CODE_SIZE>();
  BwVColumnBlock<CODE_SIZE> *block = reinterpret_cast<BwVColumnBlock<CODE_SIZE>*>(block_);
  SequentialReadFile in_file;

  status = in_file.Open("./tests/data/test_bwv");
  EXPECT_TRUE(status.IsOk());

  test_block->Load(in_file);

  EXPECT_TRUE(test_block->GetCodeSize() == block_->GetCodeSize());
  EXPECT_TRUE(test_block->GetNumCodes() == block_->GetNumCodes());
  EXPECT_TRUE(test_block->GetColumnType() == block_->GetColumnType());
  EXPECT_TRUE(test_block->num_used_words_ == block->num_used_words_);
  EXPECT_TRUE(test_block->kNumGroups == block->kNumGroups);
  for (size_t i = 0; i < block->kNumGroups; i++) {
    EXPECT_EQ(0, memcmp(test_block->data_[i], block->data_[i],
                        block->num_used_words_ * sizeof(WordUnit)));
  }

  status = in_file.Close();
  EXPECT_TRUE(status.IsOk());

  Env env;
  status = env.DeleteFile("./tests/data/test_bwv");
  EXPECT_TRUE(status.IsOk());

  delete test_block;
}

}  // namespace bitweaving
