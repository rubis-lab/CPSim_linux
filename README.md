# CPSim Project
CPSim (Cyber-Physical System Simulator) is a development tool which supports whole development process for CPSs from design to implementation.
This tool provides two essential features for development process of CPSs:

1. Functional/Temporal Co-Validation
2. Smooth Migration

[![CPSim Demo](https://github.com/rubis-lab/images/blob/master/pic2.png)](https://youtu.be/XfD0eenY6rQ "CPSim Demo")

# Prerequisite
Linux based PC (Ubuntu is recommended)
PCAN-USB (PEAK-System): this device is used to communicate between the simulator and the vehicle dynamics simulator, TORCS. 

# How to Install for Linux
##1. Install the PCAN-USB driver
You can get the driver at this link: http://www.peak-system.com/fileadmin/media/linux/index.htm#download
We recommend the version of 7.15.2(released at 29.07.2015).
Also you can use this driver with the PCAN-USB and it can be purchased at http://www.peak-system.com/PCAN-USB.199.0.html?&L=1

##2. Install Eclipse

##3. Add packages(Simulator/design/eclipse_packages/*.jar) to the folder “eclipse\dropins”

##4. Make a new workspace

##5. Copy the Simulator folder to the workspace that is created by #4

##6. Using “make” command, you can run the makefile which is a script file to compile the parser source file into executable file.

# Quick Start
##1. Connect the simulation PC which runs CPSim and the host PC which runs Torcs through PCAN-USB.
CPSim and Torcs can run at the same PC or at the seperated ones.
If this step is skipped, simulation will be fail since CPSIM and Torcs cannot communicate with each other.

##2. Run the system configurator, that is,
run eclipse with workspace that you chose in installing simulator.

##3. Create a New Project.
In order to validate a new system using CPSim, create a new project as general.
Now, you have to make “configuration file” in the project folder.
For this, right click the created project folder in the “Package Explore” tab.
Then, create a file that has extension “hxml”.

##4. Configure a whole system.
After the configuration file is created, you can find a CAN bus on the screen.
When you move your mouse on the CAN bus and right click, you can add a car selecting the menu “Add Car”.
Similarly, if you choose the menu “Add ECU” or “Remove ECU”, you can add an ECU on the system or remove from the system.
Moreover, if you right click on the ECU and choose the “Add SWC”, you can add a task.
Each component can be placed anywhere by dragging.

##5. Describe specific properties.
After configuring a whole system briefly, like step 3, describe task properties including timing parameters.
If you click on the one of the SWC, “Properties” tab might be shown at the right side of the screen.
In this tap, you can set various parameters such as ‘period’, ‘deadline’, ‘worst case execution time’, etc.
If a task uses data produced by or provides data to other component, it can be set as ‘Recv from’ or ‘Send to’ property.

##6. Run Simulator.
After all settings are done, you can run simulator as right clicking background area and choosing “Run Simulator”.

# How to Contribute

# License

# Reference
[1] Kyoung-Soo We, Chang-Gun Lee, Junyung Lee, Kyuwon Kim, Kyongsu Yi, and Jong-Chan Kim, "ECU-in-the-Loop Real-Time Simulation Technique for Developing Integrated Vehicle Safety System", in International Conference on Control, Automation and Systems (ICCAS 2014), Gyeonggi-do, Korea, Oct 2014

[2] Kyoung-Soo We, Jong-Chan Kim, Yuyeon Oh, Sangmin Jeong, and Chang-Gun Lee, "Demo Abstract: An Efficient and Easilly Reconfigurable Cyber-Physical Simulator", in International Conference on Cyber-Physical Systems (ICCPS 2013), Philadelphia, USA, Apr 2013

[3] Jae-Hwa Han, Kyoung-Soo We, and Chang-Gun Lee, "WiP Abstract: Cyber Physical Simulations for Supporting Smooth Development from All-simulated Systems to All-real Systems", in International Conference on Cyber-Physical Systems (ICCPS'12), Beijing, China, Apr 2012

[4] [Kyoung-Soo We, Jong-Chan Kim, Yuyeon Oh, and Chang-Gun Lee, "An Efficient Simulated Job Execution Algorithm for Functionally and Temporally Correct Interactions between Simulated World and Real World", in Department of Computer Science and Engineering, Seoul National University, Nov. 2013](http://rubis2.snu.ac.kr/?module=file&act=procFileDownload&file_srl=8035&sid=7a15c388401d03bd0ec5d366580c1964)
