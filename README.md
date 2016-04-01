# RF24 Arduino Transmitter Raspberry Pi Receiver

This repository is based on the [Optimized High Speed NRF24L01+ Driver Class Documenation] (http://tmrh20.github.io/RF24/index.html) sample codes. 

## Transmitter

The [MedBox_RF24_arduino_receiver.ino](https://github.com/bitandbytes/RF24-arduino-transmitter-RPi-receiver/blob/master/MedBox_RF24_arduino_receiver.ino) file is modified using the 'GettingStarted_transmitter.ino' for Arduino code sample. 

##### Added features to the GettingStarted_transmitter
* Interrupt function to the input (De-bouncing is not added and needed to be added)
* WDT to send Ping data periodically 
* Sleep mode to save power

## Receiver 
The receiver side is a Raspberry Pi with a RF24 module connected. The  GettingStarted_transmitter.cpp sample file in the [Documentation] (http://tmrh20.github.io/RF24/index.html) could be used to receive data from the RF24 module. 
