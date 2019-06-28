# Uncomment one of the lines below. 
 OPT ?= -O3    # Production use (fully optimized mode)
# OPT ?= -g     # Debug mode, with debugging symbols
# OPT ?= -O3 -g # Profiling mode, optimized mode with debugging symbols

$(shell CXX=$(CXX) ./configure.sh configure.in ./)
include configure.in

GTEST_DIR = ./third_party/gtest

CXXFLAGS += -I. -I./include -I$(GTEST_DIR)/include $(PLATFORM_CXXFLAGS) -Wall -Wno-tautological-compare $(OPT)
LDFLAGS += $(PLATFORM_LDFLAGS)

LIBOBJECTS = $(SOURCES:.cpp=.o)
TESTOBJECTS = $(TEST_SOURCES:.cpp=.o)
UNITTEST = tests/bitweaving_test

LIBRARY = libbitweaving.a

default: all

test:
	@test -s $(UNITTEST) || make
	$(UNITTEST)

doc:
	doxygen Doxyfile

clean:
	-rm -f $(LIBRARY) $(LIBOBJECTS) $(TESTOBJECTS) $(GTEST_OBJECTS) $(GTEST_LIB) $(UNITTEST)

$(LIBRARY): $(LIBOBJECTS)
	$(AR) -rs $@ $(LIBOBJECTS)

# Add $(SHARED) below if you need to genreate dynamically linked library.
all: $(LIBRARY) 
#$(UNITTEST)

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

tests/%.o: tests/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile gtest framework
GTEST_SOURCES = $(GTEST_DIR)/gtest-all.cc $(GTEST_DIR)/gtest_main.cc
GTEST_OBJECTS = $(GTEST_SOURCES:.cc=.o)
GTEST_LIB = $(GTEST_DIR)/gtest_main.a

$(GTEST_DIR)/%.o: $(GTEST_DIR)/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(GTEST_LIB) : $(GTEST_OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

$(UNITTEST): $(LIBOBJECTS) $(TESTOBJECTS) $(GTEST_LIB)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lpthread $^ -o $@
