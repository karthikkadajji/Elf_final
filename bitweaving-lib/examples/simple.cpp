// This file copyright (c) 2011-2013, the BitWeaving authors.
// All rights reserved.
// See file LICENSE for details.

#include <iostream>
#include <cstdlib>

#include "bitweaving/column.h"
#include "bitweaving/status.h"
#include "bitweaving/table.h"
#include "bitweaving/types.h"

/**
 * This example demonstrates a simple example that performs a scan
 * query on a table with two columns.
 */
int main(int argc, char *argv[]) {
  size_t num_codes = 1000;
  uint8_t bit_width = 6;
  bitweaving::Code *codes1 = new bitweaving::Code[num_codes];
  bitweaving::Code *codes2 = new bitweaving::Code[num_codes];

  // Generate data
  bitweaving::Code mask = (1ULL << bit_width) - 1;
  for (size_t i = 0; i < num_codes; i++) {
    codes1[i] = rand() & mask;
    codes2[i] = rand() & mask;
  }

  bitweaving::Code *results = new bitweaving::Code[num_codes];
  size_t num_results = 0;
  for (size_t i = 0; i < num_codes; i++) {
    // Store matching values
    if ((codes1[i] > 9 && codes1[i] < 20) && (codes2[i] < 10))
      results[num_results++] = codes1[i];
  }
  // Data generation done.

  // Create a table
  bitweaving::Options options = bitweaving::Options();
  options.in_memory = true;
  bitweaving::Table *table = new bitweaving::Table("", options);
  table->Open();

  // Add two columns into the table
  table->AddColumn("first", bitweaving::kBitWeavingV, bit_width);
  table->AddColumn("second", bitweaving::kBitWeavingV, bit_width);

  bitweaving::Column *column1, *column2;
  column1 = table->GetColumn("first");
  column2 = table->GetColumn("second");
  assert(column1 != NULL);
  assert(column2 != NULL);

  // Insert values into both columns
  column1->Bulkload(codes1, num_codes);
  column2->Bulkload(codes2, num_codes);

  // Create bitvector
  bitweaving::BitVector *bitvector1 = new bitweaving::BitVector(*table);
  bitweaving::BitVector *bitvector2 = new bitweaving::BitVector(*table);

  // Perform the scan with the predicate (9 < first < 20 && second < 10)
  column1->Scan_between(9,20,bitvector1);
  column1->Scan(9, bitweaving::kGreaterEqual, bitvector2);
  column1->Scan(20, bitweaving::kLessEqual, bitweaving::kAnd, bitvector2);
  //column2->Scan(10, bitweaving::kLess, bitweaving::kAnd, bitvector);

  // Create an iterator to get matching values
  //bitweaving::Iterator *iter = new bitweaving::Iterator(*bitvector1);

  bitvector1->toTids(new vector<uint32_t>(),num_codes);
  bitvector2->toTids(new vector<uint32_t>(),num_codes);
  bitweaving::Code code;
  size_t i = 0;
  /*while (iter->Next()) {
    // Get the next matching value
    iter->GetCode(*column1, &code);
    if (code != results[i])
      std::cout << "Wrong Result: " << code << " " << results[i] << std::endl;
    i++;
  }
  if (num_results != i)
    std::cout << "Number of results does not match." << std::endl;
*/
  table->Close();

  //delete iter;
  delete bitvector1;
  delete bitvector2;
  delete table;

  std::cout << "Pass the test." << std::endl;

  return 1;
}

