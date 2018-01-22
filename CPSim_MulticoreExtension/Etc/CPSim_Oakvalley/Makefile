CPSim_Oakvalley					: main.o Logger.o CPSimTask.o CPSimECU.o CPSimCAN.o CPSimSystem.o CPSimFunction.o CPSimMappingFunction.o
	g++ -std=c++11 -o CPSim_Oakvalley main.o Logger.o CPSimTask.o CPSimECU.o CPSimCAN.o CPSimSystem.o CPSimFunction.o CPSimMappingFunction.o
main.o									: CPSimSystem.hpp CPSimFunction.hpp CPSimMappingFunction.hpp main.cpp
	g++ -std=c++11 -c main.cpp
CPSimTask.o							: CPSimTypes.hpp CPSimTask.hpp CPSimTask.cpp
	g++ -std=c++11 -c CPSimTask.cpp
CPSimECU.o							: CPSimECU.hpp CPSimECU.cpp
	g++ -std=c++11 -c CPSimECU.cpp
CPSimCAN.o							: CPSimCAN.hpp CPSimCAN.cpp
	g++ -std=c++11 -c CPSimCAN.cpp
CPSimSystem.o						: CPSimTypes.hpp Logger.hpp CPSimTask.hpp CPSimECU.hpp CPSimCAN.hpp CPSimSystem.hpp CPSimSystem.cpp
	g++ -std=c++11 -c CPSimSystem.cpp -lpthread
CPSimFunction.o					: CPSimFunction.hpp CPSimFunction.cpp
	g++ -std=c++11 -c CPSimFunction.cpp
CPSimMappingFunction.o	: CPSimMappingFunction.hpp CPSimMappingFunction.cpp
	g++ -std=c++11 -c CPSimMappingFunction.cpp
Logger.o								: Logger.hpp Logger.cpp
	g++ -std=c++11 -c Logger.cpp -lpthread
clean										:
	rm *.o CPSim_Oakvalley
