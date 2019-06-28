// This file copyright (c) 2011-2013, the BitWeaving authors.
// All rights reserved.
// See file LICENSE for details.

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>

#include "bitweaving/column.h"
#include "bitweaving/status.h"
#include "bitweaving/table.h"
#include "bitweaving/types.h"
#include "rdtsc.h"

/**
 * This example tests the performance of some simple scan queries.
 */
int main(int argc, char* argv[]) {
  size_t num_codes = 1000;
  uint8_t bit_width = 6;
  size_t repeat = 1;
  double selectivity = 0.1;
  bitweaving::Comparator comparator = bitweaving::kLess;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-n") == 0)
      num_codes = atoi(argv[++i]);
    else if (strcmp(argv[i], "-r") == 0)
      repeat = atoi(argv[++i]);
    else if (strcmp(argv[i], "-b") == 0)
      bit_width = atoi(argv[++i]);
    else if (strcmp(argv[i], "-s") == 0)
      selectivity = atof(argv[++i]);
    else if (strcmp(argv[i], "-t") == 0) {
      i++;
      if (strcmp(argv[i], "less") == 0)
        comparator = bitweaving::kLess;
      else if (strcmp(argv[i], "greater") == 0)
        comparator = bitweaving::kGreater;
      else if (strcmp(argv[i], "equal") == 0)
        comparator = bitweaving::kEqual;
      else if (strcmp(argv[i], "inequal") == 0)
        comparator = bitweaving::kInequal;
      else
        exit(1);
    } else
      exit(1);
  }

  // Generate data
  bitweaving::Code *codes = new bitweaving::Code[num_codes];
  bitweaving::Code mask = (1ULL << bit_width) - 1;
  for (size_t i = 0; i < num_codes; i++) {
    codes[i] = rand() & mask;
  }

  // Create a table
  bitweaving::Options options = bitweaving::Options();
  options.in_memory = true;
  bitweaving::Table *table = new bitweaving::Table(std::string(), options);
  table->Open();

  // Add a column into the table
  bitweaving::Column *column;
  table->AddColumn("first", bitweaving::kBitWeavingV, bit_width);
  column = table->GetColumn("first");

  // Insert values into both columns
  column->Bulkload(codes, num_codes);

  // Create bitvector
  bitweaving::BitVector* bitvector = new bitweaving::BitVector(*table);

  typedef unsigned long long cycle;
  cycle * cycles = new cycle[repeat];
  for (size_t i = 0; i < repeat; i++) {
    cycle c;
    startTimer(&c);
    bitweaving::Code literal = static_cast<bitweaving::Code>(mask * selectivity);
    column->Scan(literal, comparator, bitvector);
    stopTimer(&c);
    cycles[i] = c;
  }

  cycle sum = 0;
  for (size_t i = 0; i < repeat; i++) {
    sum += cycles[i];
    std::cout << double(cycles[i]) / num_codes << " ";
  }
  std::cout << std::endl;
  std::cout << "Avg: " << double(sum) / repeat / num_codes << std::endl;

  table->Close();

  delete bitvector;
  delete table;
  delete[] cycles;

  return 1;
}

