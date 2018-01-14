# CPSim_linux_MulticoreExtension

## How to setup

- Install below packages and libraries using 'sudo apt-get [name]' command.

1. gcc

2. g++

3. libstdc++

4. libpopt-dev

5. openjdk-8-jre

6. openjdk-8-jdk

7. freeglut3-dev

8. libplib-dev

9. libopenal-dev

10. libalut-dev

11. libxi-dev

12. libxmu-dev

13. libxrender-dev

14. libxrandr-dev

15. zlib1g-dev

16. libpng-dev

17. libxml2-dev

18. libxslt1-dev

- Move to 'PCAN-Related/PCAN-Device-Driver' directory.

1. Decompress device driver using 'tar -xzf [name].tar.gz' command.

2. Move to decompressed directory.

3. Type 'sudo make' to build binaries.

4. Move to decompressed directory which is same with 2.

5. Type 'sudo make install' to install driver.

- Move to 'PCAN-Related/PCAN-BASIC-API' directory

1. Decompress API using 'tar -xzf [name].tar.gz' command.

2. Move to 'pcanbasic' directory in decompressed directory.

3. Type 'sudo make' to build binaries.

4. Type 'sudo make install' to install library.

- Move to 'TORCS-Related' directory.

1. Decompress Torcs using 'cat Torcs.tar.gz\* | tar xvzf -' command.

2. Move to decompressed directory.

3. Type './configure' to configure Torcs to your system.

4. Type 'sudo make' to build binaries.

5. Type 'sudo make install' to install Torcs.

6. Type 'sudo make datainstall' to install Torcs data.

- Install Eclipse as 'Eclipse IDE for C/C++ Developers'.

1. Move to 'dropins' in Eclipse installation directory.

2. Copy all of the \*.jar file in 'Simulator/design/eclipse_packages/' to the 'dropins' directory.

## 시뮬레이터 회의 (2018. 01. 14. SUN)

### 2017 RTSS (Kyoung-Soo We) Multicore Extension 개요

1. Motivation


> Thesis에 제안된 GEDF를 그대로 사용할 경우, **memory interference, Cache effect** 등의 문제로 job의 PC execution time이 unpredictable 해질 수 있음.


> 따라서, 2017 RTSS (Kyoung-Soo We)에서 제안된 execution time mapping function을 신뢰할 수 없게 될 수 있음.


> 이는 실제 ECU의 동작과는 괴리가 있는 동작을 시뮬레이션하게 될 가능성이 있음.


2. Proposal

> GEDF를 통한 multi-core 사용의 문제점을 개선하기 위하여 아래와 같은 방법을 적용.


>> SCE (Single Core Equivalence)에 제안된 **memory partitioning** 등의 방법을 사용.


>> Sangyoun Paik이 제안한(제안할?) job partitioning 방법을 사용하여 **PEDF** 사용.


> 이를 통해 얻을 수 있는 이점은 아래와 같이 생각해 볼 수 있음.


>> SCE 적용을 통해 실제 ECU의 동작과 최대한 가까운 시뮬레이션 가능(execution time mapping function 신뢰도 상승).


>> Sangyoun Paik이 제안한(제안할?) PEDF 방법을 통해 simulatability 상승(single-core 및 GEDF 대비).


3. To Do


> **Quick & Dirity >** 아직, GEDF를 사용하는 경우 생길 수 있는 문제점을 clear하게 식별하지 못했음. 따라서, GEDF로 시뮬레이션을 수행해보고 job의 수행시간에 생기는 unpredictability를 관찰해야함.


>> 문제점을 파악하는 수준의 GEDF 구현이므로 job이 preemption 된 후 resume 될 때, 이전과 다른 core에서 동작하게 되는 경우 무시.


> **Slow & Clean >** Quick & Dirty 실험 후 motivation이 명확해 지면, job이 preemption된 후 다른 core에서 동작하게 되는 경우까지 고려. SCHED_DEADLINE (RTSS 2017 TuTor) 사용 예정. 

4. To learn


> CPSim_linux - kswe branch 구동 방법 (for Quick & Dirty)


>> task code 작성 방법


>> precedence relation 명시 방법


>> WCET, BCET 주는 방법


> CPSim_linux - kswe branch Multicore extension 수정할 부분 조언 (for Quick & Dirty)


> CPSim_linux - kswe branch 코드 플로우 (for Slow & Quick)


5. 결과


1. 우선, Simulator 합치지 않은 상태에서 Quick하게 실험 코드 작성


> Single-core에서 job preemption 있는 상황.


> Multi-core에서 execution time mapping function 흔들리는 것 볼 수 있도록.

