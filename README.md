# D2A-CPSim Project
D2A-CPSim (Design to Auto-implementation for CPS with SIMulator) project was started to support convenient developing process for CPSs developers.
The tool implemented by this project provides extensive development environment from Design to Implementation.
It has three key features:
1. Functional-Temporal Co-Validation
2. Smooth Migration
3. Auto Implementation on ECU

# Prerequisite
Linux based PC (Ubuntu 12.04 LTS is recommended)
PCAN-USB (PEAK-System): this device is used to communicate between the simulator and the torcs.. 

# How to Install for Linux
##1. Install the PCAN-USB driver
You can get the driver at this link: http://www.peak-system.com/fileadmin/media/linux/index.htm#download
We recommend the version of 7.15.2(released at 29.07.2015).
Also you can use this driver with the PCAN-USB and it can be purchased at http://www.peak-system.com/PCAN-USB.199.0.html?&L=1
##2. Install Eclipse
##3. Add packages(git 폴더명 추가) to the folder “eclipse\dropins”
##4. Make a new workspace
##5. Copy the Simulator folder to the workspace that is created by #4
##6. Using “make” command, you can run the makefile which is a script file to compile the parser source file into executable file.

# Quick Start
##1. connect the Simulator PC which runs Simulator and the host PC which runs Torcs through PCAN-USB.
The Simulator and Torcs can run at the same PC or at the seperated ones.
If this step is skipped, simulation will be fail since the Simulator and Torcs cannot communicate with each other.
##2. Run the System Configurator
Run Eclipse.
If a Workspace Launcher comes on, choose the workspace that is created during the install process.
-----------System Configurator에 대한 상세 설명이 더 추가 되어야 함? JFreePlotter같은?
##3. Create a New Project
In order to validate a new system using D2A-CPSim, create a New Project as General.
Now, you have to make “Configuration File” in Project folder.
For this, right click the created project folder in the “Package Explore” tab.
Create a file that has extension “hxml” in that project folder.
##4. Configure a whole system
After the configuration file is created, you can find a CAN bus on the screen.
When you move your mouse on the CAN bus and right click, you can add a car selecting the menu “Add Car”.
Similarly, if you choose the menu “Add ECU” or “Remove ECU”, you can add an ECU on the system or remove from the system.
Moreover, if you right click on the ECU and choose the “Add SWC”, you can add a software component. 
Each component can be placed anywhere by dragging.
##5. Describe specific properties
After configuring a whole system briefly, like step 3, describe properties of each software component.
If you click on the one of the SWC, “Properties” tab might be shown at the right side of the screen.
In this tap, you can set various properties such as ‘periodic’, ‘deadline’, ‘worst case execution time’, etc.
If a software component uses data produced by or provides data to other component, it can be set as ‘Recv from’ or ‘Send to’ property.
##6. Run Simulator
After all settings are done, you can run simulator as right clicking background area and choosing “Run Simulator”.

# How to Contribute
