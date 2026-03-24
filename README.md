# INDRA - High Power Rocket Project
## Team: DJS Impulse

## Overview
INDRA is a high-power rocket developed to achieve an apogee of 1 km while carrying a functional payload and ensuring safe, autonomous recovery.

The project represents a complete rocket system, integrating avionics, structures, propulsion, and recovery into a single flight platform.

In its test flight, INDRA achieved an apogee of ~900 meters, validating the system design and onboard logic.

## Objective
- Achieve ~1 km apogee
- Test onboard payload functionality
- Ensure reliable parachute deployment
- Recover the rocket safely

## Problem Statement
Student high-power rocketry involves multiple failure points such as:
- Inaccurate flight event detection
- Unreliable recovery system triggering
- Integration challenges across subsystems
A robust system must operate reliably under high acceleration and dynamic conditions.

## Solution
INDRA uses a custom avionics-driven approach combined with a stable rocket design:

Flight Computer
- Detects launch using acceleration thresholds
- Determines apogee using filtered altitude data
- Triggers parachute deployment autonomously
- Logs flight data to SD card
Rocket System (High-Level)
- Aerodynamically stable airframe
- Solid motor propulsion selected for target altitude
- Recovery system using parachute deployment
Payload
- Independent onboard system for experimentation
- Designed to operate during flight

## Components
- Lightweight airframe - INDRA
- Solid rocket motor - AIRAVATA
- Parachute based recovery system
- Custom designed flight computer - TARA
- Custom designed payload - VAJRA

## Flight results
- Target apogee : 1000 m | Achieved apogee : ~900 m
- Launch detection successful
- Apogee detection successful
- Recovery system triggered
- Reliable and safe recovery achieved

## Repository contents
|-hardware
|-|-flight_computer_pcb.pdf
|-|-payload_pcb.pdf
|
|-software
|-|-flight_computer_tara.ino
|
|-media
|-|-INDRA_launch.mp4
|-|-INDRA_recovery.mp4

## Demo video
`link of video over here`

**INDRA demonstrates a complete, integrated approach to high-power rocketry with a focus on reliability, safety, and data-driven design.**