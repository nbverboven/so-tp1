.POSIX:
.SUFFIXES: .o .cpp

CXX = g++
CXXFLAGS = -g -std=c++11 -pthread
LDLIBS = -lpthread

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

BIN = test-concurrente
OBJ = ../entregable/ConcurrentHashMap.o

all: $(BIN)

$(BIN): ../entregable/ListaAtomica.hpp

test-concurrente: $(OBJ) test-concurrente.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ test-concurrente.cpp ConcurrentHashMap.o $(LDLIBS)
	
test-concurrente-run: test-2
	#awk -f corpus.awk corpus | sort >corpus-post
	./test-concurrente | sort | diff -u - lorem-ipsum-post
	#rm -f corpus-post


clean:
	rm -f $(BIN) $(OBJ)
	rm -f corpus-*
