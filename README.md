# ArduPilot project used for holodeck demo

## Introduction
This branch maintains the customized firmware I use for the demo in Holodeck. This branch demonstrates:
1. How to create a new MAVLink message to send Vicon data to the board.
2. How to implement an LQR controller.
3. How to use ROS to send fake MAVLink messages.

## Prerequisites
A VM that runs Ubuntu >= 16.04. 

## Build

### Build and load ArduCopter
The steps below are tested on Ubuntu 16.04.
* Clone the repository: `git clone https://github.com/dut09/ardupilot.git`
* Checkout my branch: `git checkout holodeck-demo`
* Update the submodule in that branch: `git submodule update --init --recursive`
* To install, run this script: `Tools/scripts/install-prereqs-ubuntu.sh -y`
* Then reload the path: `. ~/.profile`
* Use waf to configure the system from the root ardupilot directory: `./waf configure --board px4-v2`
* Build the copter code: `./waf copter`
* Once the build is successful, connect the Pixhawk using a USB cable, then run the following command:
```
./waf --targets bin/arducopter --upload
```

### Setup ROS to send MAVLink to the board
You need to install ROS. The code is tested with ROS Lunar.
* Follow [this link](http://wiki.ros.org/lunar/Installation/Ubuntu) to install ROS Lunar.
* Follow [this link](http://wiki.ros.org/ROS/Tutorials/InstallingandConfiguringROSEnvironment) to configure your catkin workspace correctly. 
* Install the following dependencies:
```
sudo aptitude install ros-lunar-sensor-msgs python-serial python-tz
```
* Make sure you have run `./waf copter` before you proceed. This command will automatically generate MAVLink code from XML definitions.
* Run `setup_roscopter.py` from the root folder:
```
python setup_roscopter.py
```
* Connect Pixhawk to your laptop using a USB cable.
* Now in your catkin workspace folder, run ```roscore```.
* Open a separate terminal, navigate to the catkin workspace folder, then run:
```
rosrun roscopter simulate_vicon_topic.py
```
This will create a ros node that publishes fake vicon data. It is mostly used for debugging. In the real experiments you will need to replace it with a rostopic that publishes true Vicon data.
* Next, open a seperate terminal, navigate to the catkin workspace folder, then run:
```
rosrun roscopter roscopter_node.py --device=/dev/ttyACM0 --baudrate=115200 --enable-vicon=True --vicon-name=copter
```
* If everything works well, you should see:
```
Waiting for APM heartbeat
Heartbeat from APM (system 1 component 1)
Sending all stream request for rate 10
```
Next, you should see a lot of `MocapPosition` messages in the form of:
```
name: "fake vicon data"
sample_count: XXX
translational:
  x: 4000
  y: 2000
  z: 1000
axisangle:
  x: 1.0
  y: 2.0
  z: 4.0
```

## Troubleshooting
Make sure you are using the 'default' Python on Ubuntu (`/usr/bin/python` and `/usr/bin/python3`) instead of using Anaconda, miniconda, conda, etc. This will resolve a lot of python-related build errors. You can add the following lines to your `~/.bashrc` file:
```
alias python=/usr/bin/python
alias python3=/usr/bin/python3
```
then do `source ~/.bashrc`. It's important to make sure `python` points to `python2` not `python3`. Now if you type `python` you should see something like `python 2.7.12 (Default, ...)`. Note that python2 is sufficient for a successful compilation. After setting python you should see no errors when you use the `./waf` commands for compilation.
