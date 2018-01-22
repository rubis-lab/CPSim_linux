# CPSim for Multicore Extension

This directory deals with CPSim multicore extension

## Author : {won.seok.django, ayiyowo}@gmail.com

### How to run?

- Type 'make' to build binary. The binary file named 'CPSim_Oakvalley' will be generated.

- Type './CPSim_Oakvalley' to execute binary.

### How to configure the CPSimSystem (only for Experiment 1, 2, 3)

**NOTE.** Below steps are only for Experiment 1, 2, 3 (CPSIMLINUX-6; Quick&Dirty Experiment)

- Go to **CPSimFunction.hpp** and **CPSimFunction.cpp**

> These files are related to functions which CPSimTasks executes.

> You can declare the functions of CPSimTasks at **CPSimFunction.hpp** file.

> Then, you can define the declared functions of CPSimTasks at **CPSimFunction.cpp** file.

> The functions that you define should follow below form (return type and argument).

```c

void [function_name](void* [argument_name])

```

> Above declarations and definitions are linked in **main.cpp** which is illustrated soon.

- Go to **CPSimMappingFunction.hpp** and **CPSimMappingFunction.cpp**

> These files are related to execution time mapping function which is proposed in KSWE RTSS 2017.

> You can declare & define the execution time mapping functions of CPSimTasks at **CPSimMappingFunction.hpp** and **CPSimMappingFunction.cpp**.

> The execution time mapping functions you declare and define should follow below form (return type and argument).

```c

CPSimTime_t [mapping_function_name](CPSimTime_t [argument_name])

```

> CPSimTime_t is same with unsigned long long. **Note that CPSim_Oakvalley always use nanosec as the unit of time.**

> The mapping functions that you declare and define are linked in **main.cpp** which is illustrated soon.

> The mapping functions are needed to successfully compile CPSim_Oakvalley. However, in experiment 1-3, we will not use those functions. Actually, it is unnecessary for experiment 1-3.

- Go to **main.cpp**

> In this file, you can add CPSimTasks to CPSimSystem.

> By calling *addCPSimTask* member function of CPSimSystem and allocating *CPSimTask* using *new* syntax, you can add the CPSimTask to your CPSimSystem.

> The constructor of CPSimTask has below forms.

```c

CPSimTask([name], [function], [offset], [period], [BCET], [WCET], [mapping_function])

```

> Here, note again that all of the time-related paramemters should have nanosec scale. The second and last parameters pf CPSimTask constructor are task function and execution mapping function respectively, which are you've already declared and defined at previous steps.

> In experiment 1-3, BCET and WCET parameters are not used. You can give just garbage value to them.

> You don't have to free (delete) CPSimTask explicitly. The destructor of CPSimSystem will automatically deallocate memory when CPSimSystem is destroyed.

> After adding your CPSimTasks to the CPSimSystem, you can run the CPSimSystem using *run* member function of CPSimSystem.

> In the experiment 1-3, I've already implemented configuration of Experiment 1-3 as *runExperiment1*, *runExperiment2*, *runExperiment3* member functions. This parts are hard-coded now for quick & dirty experiment. We have to implement *run* member function later when our proposed partitioning algorithm is developed.

> Below shows the execution scenarios that *runExperiment1-3* executes.

>> TODO. Experiment1

>> TODO. Experiment2

>> TODO. Experiment3

> While CPSimSystem is running, all of the logs are written at buffer which is managed by *Logger(static)* class. By calling *printLog* member function of *Logger* class, you can see the whole logs at console.