Experiment 	: main.o Error.o CPSimTask.o CPSimECU.o CPSimCAN.o CPSimSystem.o
	g++ -std=c++11 -o Experiment main.o Error.o CPSimTask.o CPSimECU.o CPSimCAN.o CPSimSystem.o -lpthread
main.o 			: CPSimSystem.hpp main.cpp
	g++ -std=c++11 -c main.cpp
CPSimTask.o : CPSimTypes.hpp CPSimTask.hpp CPSimTask.cpp
	g++ -std=c++11 -c CPSimTask.cpp
CPSimECU.o  : CPSimECU.hpp CPSimECU.cpp
	g++ -std=c++11 -c CPSimECU.cpp
CPSimCAN.o  : CPSimCAN.hpp CPSimCAN.cpp
	g++ -std=c++11 -c CPSimCAN.cpp
CPSimSystem.o : CPSimTypes.hpp Error.hpp CPSimTask.hpp CPSimECU.hpp CPSimCAN.hpp CPSimSystem.hpp CPSimSystem.cpp
	g++ -std=c++11 -c CPSimSystem.cpp -lpthread
Error.o : Error.hpp Error.cpp
	g++ -std=c++11 -c Error.cpp -lpthread
clean 			:
	rm *.o Experiment
