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

class IteratorTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    Options options = bitweaving::Options();
    options.in_memory = true;
    table_ = new Table(std::string(), options);
    code_size_ = 16;

    table_->AddColumn("a", bitweaving::kNaive, code_size_);
    Column *column = table_->GetColumn("a");

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

TEST_F(IteratorTest, ConstructorTest) {
  Iterator *iter = new Iterator(*table_);

  size_t count = 0;
  while (iter->Next()) {
    count++;
  }
  EXPECT_TRUE(count == NUM_TEST_CODES);

  delete iter;
}

}  // namespace bitweaving
