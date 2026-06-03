# Coulomb Counting Based Battery SOC Estimation

## Overview

This project implements a Coulomb Counting algorithm for real-time Battery State of Charge (SOC) estimation for Formula Bharat EV applications.

The system measures load current using the current-sense output of the NCV84045 Smart Switch, converts the sensed voltage into current using ADC measurements, and integrates the current over time to estimate the remaining battery charge and SOC.

The implementation was validated on both Arduino Uno and STM32F103C8T6 (Blue Pill) platforms.

---

## Features

* Real-time battery SOC estimation
* Current sensing using NCV84045
* ADC-based voltage acquisition
* Noise filtering for stable measurements
* Coulomb Counting charge integration
* Battery charge tracking in Ampere-seconds (As)
* Arduino Uno implementation
* STM32 Blue Pill implementation

---

## Hardware

### Battery

* Type: 12V LiFePO4 / Exide Test Battery
* Capacity: 9 Ah
* Total Charge Capacity: 32400 As

### Current Measurement

* NCV84045 Smart Switch
* 150 Ω Sense Resistor
* Current Sense Ratio: 1415
* 5 kΩ Pull-Down Resistor
* Arduino ADC Input: A0

### Microcontrollers

* Arduino Uno (ATmega328P)
* STM32F103C8T6 Blue Pill

---

## Working Principle

Coulomb Counting estimates battery charge by integrating current over time.

### Charge Calculation

Q = ∫ I(t)dt

Digital implementation:

Q = Σ(I × Δt)

### SOC Calculation

SOC(%) = (Remaining Charge / Initial Charge) × 100

---

## Software Flow

1. Read ADC value from current-sense pin.
2. Convert ADC counts to voltage.
3. Apply noise threshold filtering.
4. Calculate actual load current.
5. Measure elapsed time.
6. Compute charge consumed.
7. Update remaining battery charge.
8. Calculate SOC.
9. Display real-time values.

---

## Results

The implementation successfully:

* Measured load current using NCV84045 current sensing.
* Calculated battery charge consumption in real time.
* Estimated remaining battery charge in Ampere-seconds.
* Calculated SOC continuously.
* Demonstrated successful operation on Arduino Uno and STM32 Blue Pill platforms.

---

## Future Improvements

* Open Circuit Voltage (OCV) correction
* Temperature compensation
* Kalman Filter based SOC estimation
* EEPROM/Flash SOC storage
* CAN Bus integration
* Complete BMS integration

---

## Applications

* Formula Bharat EV
* Battery Management Systems (BMS)
* Power Distribution Modules (PDM)
* Battery Health Monitoring
* Embedded Energy Management Systems

---

## Authors

*Sanchit Joshi
*Rishit Shetty

Team Octane Racing Electric
