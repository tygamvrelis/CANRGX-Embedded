@mainpage CAN-RGX - Embedded Systems for Team FAM


# Introduction
Convection is a common phenomenon behind many of the heat transfer processes on Earth. It is driven by the buoyant force resulting from the density differences of a fluid with temperature. In microgravity, there is no buoyant force and thus no convection. This poses challenges for conventional heat transfer methods.

One approach to this problem involves using parafluids (fluids that respond to magnetism) under the influence of external magnetic fields to artificially introduce convection. Our experiment, which uses a Schlieren imaging system to visualize temperature gradients, has been designed to study this approach.


<img src="../../images/equipment/all_2.JPG" width="40%" hspace="20" vspace="20" style="float:left" />
<img src="../../images/equipment/all_3.JPG" width="40%" hspace="20" vspace="20" style="float:left" />
<img src="../../images/equipment/optics_1.JPG" width="40%" hspace="20" vspace="20" style="float:left" />
<img src="../../images/equipment/optics_2.JPG" width="40%" hspace="20" vspace="20" style="float:left" />
<div style="clear:left"></div>


The embedded systems are responsible for:
-# Collecting sensor data
-# Generating control signals
-# Starting and stopping experiments, through both manual and automated mechanisms
-# PC communication for streaming back data and receiving commands


# High-level Overview
A STM32F446RE microcontroller on a Nucleo-F446RE was chosen as the platform for the embedded systems. The software runs ontop of FreeRTOS, a real-time OS with a priority-based preemptive scheduler. All IO is DMA-based where possible, and interrupt-based otherwise. The peripherals involved are:
- MPU9250 inertial measurement unit and magnetometer
- 2-wire PWM drivers for the magnets (x2)
- 1-wire PWM drivers for the TECs (x2)
- analog temperature sensors (x6)
- LED for camera synchronization
- LED for indicating microcontroller status (integrated with development board)


## Threads
The software consists of 6 threads (note: priority = 6 is the highest priority while priority = 0 is the lowest priority):
-# **PC RX, priority 6**: event-based, receives command packets from PC 
-# **PC TX, priority 5**: time-triggered, sends packets to the PC consisting of sensor data and controller state information 
-# **Control, priority 3**: time-triggered, contains the control signal logic used for generating the various experiment conditions for each parabola
-# **MPU9250, priority 3**: time-triggered, reads acceleration and magnetic flux density from the MPU9250 sensor. Also filters acceleration data, and generates microgravity transition events for the control thread depending on the readings
-# **Temp, priority 3**: time-triggered, caches the asyncronously-acquired and processed temperature data for transmission
-# **default, priority 0**: CubeMX-generated function that immediately sleeps


## PC Commands
The PC interface supports 3 commands, which are treated as manual overrides:
-# **START_EXPERIMENT**: starts the specified experiment (passed as an argument) by forcing a controller state transition
-# **STOP_EXPERIMENT**: stops the currently-running experiment by forcing a controller state transition
-# **RESET_MCU**: sets control signals to a safe (idle) state, waits up to 1 second for I/O transfers to finish, then performs a hard reset of the MCU


## Principle of Operation
The image below shows some of the possible interactions between the components of the system once the scheduler has started.


<img src="../../images/design/Flight_1_Design.jpg" width="80%" alt="Steady-state flow">


### PC Communication
Once the scheduler begins, the RX thread runs. It initiates a reception then immediately blocks until a command packet is received or the waiting times out (in which case the receive buffer is cleared and a new reception is initiated). Time-triggering the data transmission thread results in packets being sent at a deterministic frequency, whose inverse is a multiple of the OS tick period (i.e. an integer number of milliseconds). Specifically, a packet is transmitted every 3 ms. This leads not only to improved system reliability—since there are no conditions that have to be met for packet transmission besides timing—but also, it eases data analysis since all points are lined up with integer time indices. While the RX and RX threads are not running, the data acquisition and control routines execute.


### MPU9250
MPU9250 data is collected every 2 ms. The MPU9250 is actually a pair of sensors—one a magnetometer, and the other an accelerometer and gyroscope—which are accessed via separate I2C busses. Acceleration is filtered along each axis with a 21-tap FIR LPF. After filtering, the total acceleration is computed and compared to pre-determined thresholds for detecting microgravity transition events.
-# Entering reduced gravity: A < 0.981 [m/s^2]
-# Leaving reduced gravity: A > 3.13 [m/s^2]


### Temperature sensors
The temperature sensors output analog signals which are quantized by 12-bit ADCs on the microcontroller. Samples are asynchronously buffered using DMA, then block-averaged in the buffer full/half-full callbacks. ADC data is cached for transmission every 10 ms.


### Control
The control thread is run every 2 ms to check for flight events and update control signals. The set of flight events consists of microgravity transition events sent from the MPU9250 thread, and experiment start and stop commands (manual overrides) sent from the RX thread. When a transition into microgravity conditions are detected by the accelerometer, or an experiment start command is received, a one-time "experiment start" state update is executed. The details of this one-time update depend on the experiment conditions which are to be created, but in general it may include turning on the TECs and/or setting the magnet drivers to PWM mode. This one-time update also always blinks the camera synchronization LED for 100 ms (about 3 frames) and increases the status LED blinking frequency to 10 Hz. After an experiment has been started, the main job of the control thread is to update the PWM signals for the magnets accoring to (in general) time-varying functions. When a transition out of microgravity conditions are detected by the accelerometer, or an experiment stop command is received, the control signals are set to a safe (idle) state and the next controller state is updated.
