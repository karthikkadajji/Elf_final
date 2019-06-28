# Uncomment exactly one of the lines below. 
# OPT ?= -O3    # Production use (fully optimized mode)
OPT ?= -g     # Debug mode, with debugging symbols
# OPT ?= -O3 -g # Profiling mode, optimized mode with debugging symbols

CXXFLAGS += -I../include -Wall $(OPT)

EXAMPLES = \
	simple \
	speed

LIBRARY = ../libbitweaving.a

default: all

clean:
	-rm -f $(EXAMPLES) simple.o speed.o

all: $(LIBRARY) $(EXAMPLES)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

simple: simple.o
	@test -s $(LIBRARY) || $(MAKE) all -C .. 
	$(CXX) $(CXXFLAGS) $(LDFLAGS) simple.o $(LIBRARY) -o $@ $(LIBS)

speed: speed.o 
	@test -s $(LIBRARY) || $(MAKE) all -C .. 
	$(CXX) $(CXXFLAGS) $(LDFLAGS) speed.o $(LIBRARY) -o $@ $(LIBS)

