@mainpage CAN-RGX - Embedded Systems for Team FAM

@section Introduction
Convection is a common phenomenon behind many of the heat transfer processes on Earth. It is driven by the buoyant force resulting from the density differences of a fluid with temperature. In microgravity, there is no buoyant force and thus no convection. This poses challenges for conventional heat transfer methods.

One approach to this problem involves using parafluids (fluids that respond to magnetism) under the influence of external magnetic fields to artificially introduce convection. Our experiment, which uses a Schlieren imaging system to visualize temperature gradients, has been designed to study this approach.

<!-- TODO: Add pictures of our experiment here -->

The embedded systems are responsible for:
-# Collecting sensor data
-# Generating control signals
-# Starting and stopping experiments, through both manual and automated mechanisms
-# PC communication for streaming back data and receiving commands

A STM32F446RE microcontroller on a Nucleo-F446RE is used to accomplish this. The software runs ontop of FreeRTOS, a real-time OS with a priority-based preemptive scheduler. All IO is DMA-based where possible, and interrupt-based otherwise.

@section High-level Overview
Coming soon
<img src="../../images/Flight_1_Design.jpg" width="80%" alt="Steady-state flow">
