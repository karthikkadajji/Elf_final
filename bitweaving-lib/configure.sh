#!/bin/sh

OUTPUT=$1
PREFIX=$2
if test -z "$OUTPUT" || test -z "$PREFIX"; then
  echo "usage: $0 <output-filename> <directory_prefix>" >&2
  exit 1
fi

# Delete existing output file
rm -f $OUTPUT
touch $OUTPUT

if test -z "$CXX"; then
  CXX=g++
fi

PLATFORM_CXXFLAGS=" "

# Make a list for all cpp files
DIRS="$PREFIX/src"
TESTDIRS="$PREFIX/tests"
EXAMPLEDIRS="$PREFIX/examples"

# Check if popcnt instruction is supported
#if grep -q popcnt /proc/cpuinfo; then
#PLATFORM_CXXFLAGS+="-DPOPCNT"
#fi

set -f
SOURCE_FILES=`find $DIRS -name '*.cpp' -print | sort | sed "s,^$PREFIX/,," | tr "\n" " "`
TEST_FILES=`find $TESTDIRS -name '*.cpp' -print | sort | sed "s,^$PREFIX/,," | tr "\n" " "`
EXAMPLE_FILES=`find $EXAMPLEDIRS -name '*.cpp' -print | sort | sed "s,^$PREFIX/,," | tr "\n" " "`
set +f

echo "SOURCES=$SOURCE_FILES" >> $OUTPUT
echo "TEST_SOURCES=$TEST_FILES" >> $OUTPUT
echo "EXAMPLE_SOURCES=$EXAMPLE_FILES" >> $OUTPUT
echo "CXX=$CXX" >> $OUTPUT
echo "PLATFORM_CXXFLAGS=-DPOPCNT" >> $OUTPUT

