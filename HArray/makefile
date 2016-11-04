OBJS= HArrayVarRAM_delValueByKey.o HArrayVarRAM_getKeysAndValuesByRange.o HArrayVarRAM_getValueByKey.o HArrayVarRAM_getValuesByRange.o HArrayVarRAM_hasPartKey.o HArrayVarRAM_insert.o HArrayVarRAM_scanKeysAndValues.o Main.o stdafx.o

CXXFLAGS= -O -pipe -Wall -Wno-write-strings -std=c++11

test: $(OBJS)
        $(CXX) -o test $(OBJS)

.cpp.o:
        $(CXX) -c $< $(CXXFLAGS)

clean:
        rm -f test ./*.o

.PHONY: clean
