Experiment 	: main.o CPSimTask.o CPSimECU.o CPSimCAN.o
	g++ -std=c++11 -o Experiment main.o CPSimTask.o CPSimECU.o CPSimCAN.o -lpthread
main.o 			: CPSimTypes.hpp CPSimTask.hpp CPSimECU.hpp CPSimCAN.hpp main.cpp
	g++ -std=c++11 -c main.cpp -lpthread
CPSimTask.o : CPSimTypes.hpp CPSimTask.hpp CPSimTask.cpp
	g++ -std=c++11 -c CPSimTask.cpp -lpthread
CPSimECU.o  : CPSimECU.hpp CPSimECU.cpp
	g++ -std=c++11 -c CPSimECU.cpp -lpthread
CPSimCAN.o  : CPSimCAN.hpp CPSimCAN.cpp
	g++ -std=c++11 -c CPSimCAN.cpp -lpthread
clean 			:
	rm *.o Experiment
