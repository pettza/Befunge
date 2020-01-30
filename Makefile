.PHONY: clean distclean default

CXX=c++
CXXFLAGS=-Wall -std=c++11

default: CXXFLAGS += -O2
default: befunge93+

debug: CXXFLAGS += -g
debug: befunge93+

*.o: *.cpp

befunge93+: bef_type.o stack.o heap.o befunge93+.o
	$(CXX) $(CXXFLAGS) -o befunge93+ $^

clean:
	$(RM) befunge93+.o heap.o stack.o

distclean: clean
	$(RM) befunge93+