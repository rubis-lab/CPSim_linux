CPPFLAGS=-g -D_REENTRANT -pthread
LDFLAGS=-g
LDLIBS=-lm -ldl -lpthread


generating_codes: generating_codes.o
	g++ $(LDFLAGS) -o generating_codes generating_codes.o $(LDLIBS)

generating_codes.o: generating_codes.cpp
	g++ $(CPPFLAGS) -c generating_codes.cpp

clean:
	rm -rf generating_codes generating_codes.o
