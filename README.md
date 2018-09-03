# CANRGX-Embedded

## \_old
Code used in early stages of development. These are a blank CubeMX project to illustrate pin assignments as of April 7, 2018, and ADC + serial code running on top of FreeRTOS

## doc
Various documentation, including an interactive HTML for the embedded program (generated using Doxygen), circuit schematics for the electrical systems, and a few other tidbits

## MPUTest
The "master" embedded program, which is used in the actual experiment

## PastFlightData
The "Flight_1_August_30_2018" subdirectory contains raw data + visualizations for our flight on August 30, 2018
The "old_acceleration_data_from_nrc" subdirectory features raw acceleration data and the corresponding plots from past flights, provided by the NRC

## PC-Side
Real-time GUI which plots sensor data and displays the controller state. The GUI also features manual override buttons for starting and stopping experiments, and in extreme cases resetting the microcontroller