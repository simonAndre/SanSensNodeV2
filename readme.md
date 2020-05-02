# SanSensNodeV2 for Arduino sensor node 

This library provides all the plumbing to manage and exchange sensor data between light arduino clients and server over MQTT 

based on : 

+ [pubsubclient](https://github.com/knolleary/pubsubclient.git)
+ [ArduinoJson](https://github.com/bblanchon/ArduinoJson.git)
+ [mpaland/printf](https://github.com/mpaland/printf)
+ [arduino-logger](https://github.com/embeddedartistry/arduino-logger.git)
+ [simonAndre/consoleMenu](git@github.com:simonAndre/consoleMenu.git)

## installation

To clone this project, use the `--recurse-submodules` option to get all the dependant libraries :  
```git
git clone --recurse-submodules git@github.com:simonAndre/SanSensNodeV2.git
```


### Prerequisite

you need to install the following libraries (from Arduino libraries menu) :
+ [simonAndre/consoleMenu](git@github.com:simonAndre/consoleMenu.git)
+ [PubSubClient](https://github.com/knolleary/pubsubclient.git) 
+ [ArduinoJson](https://github.com/bblanchon/ArduinoJson.git)
+ [LibPrintf](https://github.com/embeddedartistry/arduino-printf)
+ [FlexLogger](https://github.com/embeddedartistry/arduino-logger.git)

all the other dependant libraries are included in submodules


WORK IN PROGRESS

## Examples

The library comes with a number of example sketches. See File > Examples
within the Arduino application.

## how it works

## mqtt message commands

this commands can be extended in your sketch.
This list is managed by the library itself : 
+ `sleep` (type:bool, default:true) : Put the node in sleep mode after each data collection
+ `serial` (type:bool, default:true) : activate or disable the serial log output 
+ `G` (type:int, default:60) : duration between each measurement (in seconds) .
+ `P` : publication factor : multiple of G. data are published and mqtt commands are retrived at each P (a higer P, a better energy optimisation)
**to complete**


## Compatible Hardware


 - Arduino Ethernet
 - ESP32

## License

This code is released under the MIT License.