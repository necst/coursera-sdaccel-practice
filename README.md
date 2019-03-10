# Developing FPGA-accelerated cloud applications with SDAccel: Practice - Course Material

## Introduction
This repository hosts the source codes used during the Coursera course titled "Developing FPGA-accelerated cloud applications with SDAccel: Practice"

## Software Requirements
All this codes have been tested using Xilinx SDAccel 2017.2 on a Linux Machine. Software functionality is not guaranteed with different versions of the tools.

## Folder Structure
Each folder is organized following the organization of the specialization course in Coursera.
There is a week number, that containes multiple modules. Each module containes multiple lessons and each lesson presents multiple items.
For each item, we provide one or multiple source codes.
Overall, the folder organization is as follows:
``` 
|-- <week-number>
    |-- <module_number>_<module-name>
        |-- <lesson_number>_<lesson-name>
            |-- <item_number>_<item-name>
                |-- source_codes
```

# Code Usage
The step-by-step description on how to launch the tools and develop the application is explained in the videos available in Coursera.
It is also possilbe to jump at a specific lesson and test a specific code. 
Here there are the general steps to follow to integrate a code and launch software emulation:
``` 
1. Open Xilinx SDAccel 2017.2
2. Select a specific workspace (unless you specified to not ask again)
3. File -> new -> SDx Project -> Application Project
4. Provide a name and location for the Project
5. Select a target board and complete the successive steps selecting a blank project
6. Right click on the src/ folder -> import -> General -> File System
7. Navigate to the specifc directory you want to integrate and select the source files
8. select project.sdx in the Project Explorer tab
9. Click on the thunder icon to Add Hardware Function and select the "compute matrices" function (it may be the only one suggested)
10. Build the project for Emulation CPU to check if everything is correct
11. Run -> Run Configuration and select the entry under "OpenCL"
12. Click on Arguments and check "Automatically add binary container(s) to arguments" to pass the generated binary to the host program
13. Click on Apply and Run
```