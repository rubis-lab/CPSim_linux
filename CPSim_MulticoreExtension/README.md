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
