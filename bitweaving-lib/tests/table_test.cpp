// This file copyright (c) 2011-2013, the BitWeaving authors.
// All rights reserved.
// See file LICENSE for details.
#include <string>

#include "bitweaving/iterator.h"
#include "bitweaving/table.h"
#include "bitweaving/types.h"
#include "gtest/gtest.h"
#include "src/env.h"
#include "src/file.h"

namespace bitweaving {

class TableTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    Options options = bitweaving::Options();
    options.in_memory = true;
    table_ = new Table(std::string(), options);
  }

  virtual void TearDown() {
    delete table_;
  }

  Table* table_;
};

TEST_F(TableTest, AddRemoveColumns) {
  size_t bit_width = 6;
  Status status;

  status = table_->AddColumn("first", bitweaving::kNaive, bit_width);
  EXPECT_TRUE(status.IsOk());
  status = table_->AddColumn("second", bitweaving::kNaive, bit_width);
  EXPECT_TRUE(status.IsOk());
  status = table_->AddColumn("third", bitweaving::kNaive, bit_width);
  EXPECT_TRUE(status.IsOk());

  bitweaving::Column* column1, *column2, *column3;
  column1 = table_->GetColumn("first");
  EXPECT_TRUE(column1 != NULL);
  column2 = table_->GetColumn("second");
  EXPECT_TRUE(column2 != NULL);
  column3 = table_->GetColumn("third");
  EXPECT_TRUE(column3 != NULL);

  EXPECT_EQ(column1->GetColumnId(), (ColumnId )0);
  EXPECT_EQ(column2->GetColumnId(), (ColumnId )1);
  EXPECT_EQ(column3->GetColumnId(), (ColumnId )2);

  // Remove first and second columns
  status = table_->RemoveColumn("second");
  EXPECT_TRUE(status.IsOk());
  status = table_->RemoveColumn("first");
  EXPECT_TRUE(status.IsOk());
  column2 = table_->GetColumn("second");
  EXPECT_FALSE(column2 != NULL);
  column1 = table_->GetColumn("first");
  EXPECT_FALSE(column1 != NULL);

  // Add two more columns
  status = table_->AddColumn("fourth", bitweaving::kNaive, bit_width);
  EXPECT_TRUE(status.IsOk());
  status = table_->AddColumn("fifth", bitweaving::kNaive, bit_width);
  EXPECT_TRUE(status.IsOk());
  status = table_->AddColumn("sixth", bitweaving::kNaive, bit_width);
  EXPECT_TRUE(status.IsOk());
  bitweaving::Column* column4, *column5, *column6;
  column4 = table_->GetColumn("fourth");
  EXPECT_TRUE(column4 != NULL);
  column5 = table_->GetColumn("fifth");
  EXPECT_TRUE(column5 != NULL);
  column6 = table_->GetColumn("sixth");
  EXPECT_TRUE(column6 != NULL);

  EXPECT_EQ(column4->GetColumnId(), (ColumnId )1);
  EXPECT_EQ(column5->GetColumnId(), (ColumnId )0);
  EXPECT_EQ(column6->GetColumnId(), (ColumnId )3);
}

TEST_F(TableTest, SaveLoadTableTest) {
  Status status;
  const std::string table_path("./tests/data/test_table");
  Options options = bitweaving::Options();
  options.delete_exist_files = true;
  Table *table = new Table(table_path, options);
  status = table->Open();
  EXPECT_TRUE(status.IsOk());

  status = table->AddColumn("first", bitweaving::kNaive, 32);
  EXPECT_TRUE(status.IsOk());
  status = table->AddColumn("second", bitweaving::kNaive, 32);
  EXPECT_TRUE(status.IsOk());

  size_t num_tuples = 100;
  Code *codes = new Code[num_tuples];
  for (size_t i = 0; i < num_tuples; i++) {
    codes[i] = 142857;
  }
  Column *column1 = table->GetColumn("first");
  column1->Bulkload(codes, num_tuples);
  for (size_t i = 0; i < num_tuples; i++) {
    codes[i] = 7;
  }
  Column *column2 = table->GetColumn("second");
  column2->Bulkload(codes, num_tuples);

  BitVector *bitvector = new BitVector(*table);
  status = column1->Scan(142857, kEqual, bitvector);
  EXPECT_TRUE(status.IsOk());
  EXPECT_EQ(num_tuples, bitvector->CountOnes());

  status = column1->Scan(7, kEqual, bitvector);
  EXPECT_TRUE(status.IsOk());
  EXPECT_TRUE(0 == bitvector->CountOnes());

  status = column2->Scan(7, kEqual, bitvector);
  EXPECT_TRUE(status.IsOk());
  EXPECT_EQ(num_tuples, bitvector->CountOnes());

  status = table->Save();
  EXPECT_TRUE(status.IsOk());

  table->Close();
  delete table;

  options.delete_exist_files = false;
  table = new Table(table_path, options);
  status = table->Open();
  EXPECT_TRUE(status.IsOk());

  EXPECT_EQ(num_tuples, table->GetNumTuples());

  status = column1->Scan(142857, kEqual, bitvector);
  EXPECT_TRUE(status.IsOk());
  EXPECT_EQ(num_tuples, bitvector->CountOnes());

  status = column1->Scan(7, kEqual, bitvector);
  EXPECT_TRUE(status.IsOk());
  EXPECT_TRUE(0 == bitvector->CountOnes());

  status = column2->Scan(7, kEqual, bitvector);
  EXPECT_TRUE(status.IsOk());
  EXPECT_EQ(num_tuples, bitvector->CountOnes());

  status = table->Delete();
  EXPECT_TRUE(status.IsOk());
  Env env;
  EXPECT_FALSE(env.IsDirectoryExist(table_path));

  table->Close();
  delete table;
}

TEST_F(TableTest, OpenLongPathTableTest) {
  Status status;
  Env env;
  const std::string table_path("./tests/data/dir1/dir2/test");
  Options options = bitweaving::Options();
  Table *table = new Table(table_path, options);
  status = table->Open();
  EXPECT_TRUE(status.IsOk());
  EXPECT_TRUE(env.IsDirectoryExist("./tests/data/dir1/dir2/test"));

  status = env.DeleteDirectory("./tests/data/dir1");
  EXPECT_TRUE(status.IsOk());
  EXPECT_FALSE(env.IsDirectoryExist("./tests/data/dir1"));
}

TEST_F(TableTest, DeleteExistFileTest) {
  Status status;
  const std::string table_path("./tests/data/test_table");
  Options options = bitweaving::Options();
  options.delete_exist_files = true;
  Table *table = new Table(table_path, options);
  status = table->Open();
  EXPECT_TRUE(status.IsOk());

  status = table->AddColumn("first", bitweaving::kNaive, 32);
  EXPECT_TRUE(status.IsOk());

  size_t num_tuples = 100;
  Code *codes = new Code[num_tuples];
  for (size_t i = 0; i < num_tuples; i++) {
    codes[i] = 142857;
  }
  Column *column = table->GetColumn("first");
  column->Bulkload(codes, num_tuples);

  status = table->Save();
  EXPECT_TRUE(status.IsOk());
  table->Close();
  delete table;

  options.delete_exist_files = false;
  table = new Table(table_path, options);
  status = table->Open();
  EXPECT_TRUE(status.IsOk());
  EXPECT_EQ(num_tuples, table->GetNumTuples());
  table->Close();
  delete table;

  options.delete_exist_files = true;
  table = new Table(table_path, options);
  status = table->Open();
  EXPECT_TRUE(status.IsOk());
  EXPECT_TRUE(0 == table->GetNumTuples());
  table->Close();
  delete table;

  Env env;
  status = env.DeleteDirectory("./tests/data/test_table");
  EXPECT_TRUE(status.IsOk());
}

// TODO: need to add more unit tests. Say, append first then write more.
TEST_F(TableTest, ResizeTest) {
  size_t bit_width = 32;
  Status status;
  status = table_->AddColumn("first", bitweaving::kNaive, bit_width);
  EXPECT_TRUE(status.IsOk());
  status = table_->AddColumn("second", bitweaving::kNaive, bit_width);
  EXPECT_TRUE(status.IsOk());

  size_t num_tuples = 100;
  table_->Resize(num_tuples);

  Iterator *iterator = new Iterator(*table_);
  iterator->Begin();
  Column *column1 = table_->GetColumn("first");
  size_t num = 0;
  while (iterator->Next()) {
    status = iterator->SetCode(*column1, 142857);
    EXPECT_TRUE(status.IsOk());
    num++;
  }
  EXPECT_EQ(num_tuples, num);

  Code *codes = new Code[num_tuples];
  for (size_t i = 0; i < num_tuples; i++) {
    codes[i] = 7;
  }
  Column *column2 = table_->GetColumn("second");
  status = column2->Bulkload(codes, num_tuples);
  EXPECT_TRUE(status.IsOk());

  BitVector *bitvector = new BitVector(*table_);
  status = column1->Scan(142857, kEqual, bitvector);
  EXPECT_TRUE(status.IsOk());
  EXPECT_EQ(num_tuples, bitvector->CountOnes());

  status = column2->Scan(7, kEqual, bitvector);
  EXPECT_TRUE(status.IsOk());
  EXPECT_EQ(num_tuples, bitvector->CountOnes());

  delete iterator;
  delete column1;
  delete column2;
  delete bitvector;
  delete []codes;
}

TEST_F(TableTest, BulkLoadTest) {
  size_t bit_width = 32;
  Status status;
  status = table_->AddColumn("first", bitweaving::kNaive, bit_width);
  status = table_->AddColumn("second", bitweaving::kNaive, bit_width);
  EXPECT_TRUE(status.IsOk());

  size_t num_tuples = kNumCodesPerBlock * 2 + 1234;
  table_->Resize(num_tuples);

  Code *codes = new Code[num_tuples];
  for (size_t i = 0; i < num_tuples; i++) {
    codes[i] = 7;
  }

  Column *column1 = table_->GetColumn("first");
  status = column1->Bulkload(codes, num_tuples);
  EXPECT_TRUE(status.IsOk());

  TupleId offset = 0;
  Column *column2 = table_->GetColumn("second");
  while (num_tuples - offset > 0) {
    size_t num = std::min(num_tuples - offset, kNumCodesPerBlock / 4 + 1000);
    status = column2->Bulkload(codes + offset, num, offset);
    EXPECT_TRUE(status.IsOk());
    offset += num;
  }

  BitVector *bitvector = new BitVector(*table_);
  status = column1->Scan(7, kEqual, bitvector);
  EXPECT_TRUE(status.IsOk());
  EXPECT_EQ(num_tuples, bitvector->CountOnes());

  status = column2->Scan(7, kEqual, bitvector);
  EXPECT_TRUE(status.IsOk());
  EXPECT_EQ(num_tuples, bitvector->CountOnes());

  delete bitvector;
  delete []codes;
}

TEST_F(TableTest, IncrementalBulkLoadTest) {
  size_t bit_width = 32;
  Status status;
  status = table_->AddColumn("first", bitweaving::kNaive, bit_width);
  status = table_->AddColumn("second", bitweaving::kNaive, bit_width);
  EXPECT_TRUE(status.IsOk());

  size_t num_tuples = kNumCodesPerBlock * 2 + 1234;
  table_->Resize(num_tuples);

  Code *codes = new Code[num_tuples];
  for (size_t i = 0; i < num_tuples; i++) {
    codes[i] = 7;
  }

  TupleId offset = 0;
  Column *column1 = table_->GetColumn("first");
  while (num_tuples - offset > 0) {
    size_t num = std::min(num_tuples - offset, kNumCodesPerBlock / 4 + 1000);
    status = column1->Bulkload(codes + offset, num, offset);
    EXPECT_TRUE(status.IsOk());
    offset += num;
  }

  Column *column2 = table_->GetColumn("second");
  status = column2->Bulkload(codes, num_tuples);
  EXPECT_TRUE(status.IsOk());

  BitVector *bitvector = new BitVector(*table_);
  status = column1->Scan(7, kEqual, bitvector);
  EXPECT_TRUE(status.IsOk());
  EXPECT_EQ(num_tuples, bitvector->CountOnes());

  status = column2->Scan(7, kEqual, bitvector);
  EXPECT_TRUE(status.IsOk());
  EXPECT_EQ(num_tuples, bitvector->CountOnes());

  delete bitvector;
  delete []codes;
}

}  // namespace bitweaving
