# Coulomb-Counting-For-SOC-Calculation-Using-Arduino-Uno-STM32F103C8T6
A MCU-based Coulomb Counting system for real-time battery (SOC) estimation in Formula Bharat EV applications. Implements current sensing, ADC acquisition, and charge integration to track  SOC. Developed and validated on Arduino Uno and STM32 Blue Pill platforms for integration into custom Power Distribution Module (PDM) &amp; (BMS) architectures.
Coulomb Counting Based Battery SOC Estimation
Overview

This project implements a Coulomb Counting algorithm for real-time Battery State of Charge (SOC) estimation. The system continuously measures battery current, calculates the charge consumed over time, and estimates the remaining battery capacity and SOC.

The project was developed for Formula Bharat EV applications and validated on both Arduino Uno and STM32 Blue Pill platforms for integration into a custom Power Distribution Module (PDM) and Battery Management System (BMS).

Features
Real-time battery charge tracking
State of Charge (SOC) estimation
Current measurement using NCV84045 current-sense output
ADC-based data acquisition
Noise filtering for stable measurements
Charge integration using Coulomb Counting
Arduino Uno implementation
STM32 Blue Pill implementation
Suitable for EV battery monitoring applications
Hardware Used
Battery
Exide 12V Lead Acid Battery
Capacity: 9 Ah
Microcontrollers
Arduino Uno (ATmega328P)
STM32F103C8T6 Blue Pill
Current Sensing
NCV84045 Smart Switch
150 Ω Sense Resistor
Working Principle

Coulomb Counting estimates battery charge by integrating current over time.

Charge Calculation
Q=∫I(t)dt

Where:

Q = Charge transferred
I(t) = Battery current
t = Time

For digital implementation:

Q=∑I
k
	​

Δt
State of Charge Calculation
SOC=
Q
initial
	​

Q
remaining
	​

	​

×100

The measured current is sampled periodically through the ADC and integrated to determine the remaining battery charge.

Arduino Uno Implementation
Specifications
Parameter	Value
ADC Resolution	10-bit
Reference Voltage	5 V
Sampling Method	millis()
Current Sense Input	A0
Current Calculation
I=
R
SENSE
	​

V
CS
	​

	​

×K
CS
	​


Where:

R
SENSE
	​

=150Ω
K
CS
	​

=1415

A noise threshold of 15 mV is applied to reduce false current measurements.

STM32 Blue Pill Implementation
Specifications
Parameter	Value
MCU	STM32F103C8T6
Clock Frequency	72 MHz
ADC Resolution	12-bit
Development Environment	STM32CubeIDE

The STM32 implementation provides higher processing capability and improved timing precision, making it suitable for advanced BMS applications.

Results

The implemented Coulomb Counter successfully:

Measured load current in real time
Calculated consumed charge
Estimated remaining battery capacity
Calculated battery SOC continuously
Demonstrated operation on Arduino Uno and STM32 Blue Pill platforms
Applications
Formula Bharat EV
Battery Management Systems (BMS)
Power Distribution Modules (PDM)
Electric Vehicle Battery Monitoring
Energy Storage Systems
Future Improvements
Open Circuit Voltage (OCV) based SOC correction
Temperature compensation
Kalman Filter based estimation
Data logging
CAN Bus integration
Integration with full EV BMS architecture
Authors

Developed as part of Formula Bharat EV Electronics Development for Battery Monitoring and Power Distribution applications.
