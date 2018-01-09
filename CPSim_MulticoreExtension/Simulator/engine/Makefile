CPPFLAGS=-g -D_REENTRANT -Wno-write-strings -pthread -lpcan
LDFLAGS=-g
LDLIBS=-lm -ldl -lpthread -lpcan


real-time_simulator: *.o
	g++ $(LDFLAGS) -o real-time_simulator *.o $(LDLIBS)

*.o: *.cpp
	g++ $(CPPFLAGS) -c *.cpp

clean:
	rm -rf *.o real-time_simulator
