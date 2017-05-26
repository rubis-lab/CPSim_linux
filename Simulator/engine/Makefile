CPPFLAGS=-g -DNOCANMODE -D_REENTRANT -Wno-write-strings -pthread
LDFLAGS=-g
LDLIBS=-lm -ldl -lpthread


real-time_simulator: *.o
	g++ $(LDFLAGS) -o real-time_simulator *.o $(LDLIBS)

*.o: *.cpp
	g++ $(CPPFLAGS) -c *.cpp

clean:
	rm -rf *.o real-time_simulator *.hh task_created.cpp
