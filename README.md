# Development of a Multifunctional Portable Printer Robot

## Overview
This project focuses on the development of a multifunctional portable printer robot designed for lightweight and efficient printing. The printer robot aims to enhance ease of use and portability of printing, making it suitable for various applications, including on the go printing tasks or professional printing on mutliple different sized paper

## Team
- **Sean Glendinning**
- **Fraser Holman**
- **Caelan Mackenzie**
- **Jay Maltman**

## Supervisors
- **Dr. Cheng Wang**
- **Xie Dong**

## Universities
**Heriot-Watt University, Edinburgh, United Kingdom**  
**Edinburgh University, Edinburgh, United Kingdom**

**Degree**: Master of Engineering in Robotics, Autonomous and Interactive Systems

## App Features
- **Bluetooth Connectivity**: Allows the printer to connect with mobile devices for easy operation.
- **Image Processing**: Takes images, converts them to bitmap format, and preprocesses them (e.g., removes large white spaces, detects grey areas).
- **User-Friendly Interface**: A mobile application that allows users to control the printer and manage print jobs.

## MCU Features
- **Bluetooth Connectivity**: Allows the printer to connect with mobile devices for easy operation.
- **Printer Control**: Safe control of the inkjet printer module.
- **Mapping**: Ability to interpret files as a bitmap and succesfully map out and traverse required area.

## Hardware Features
- **Lightweight Design**: The printer robot is designed to be lightweight, making it portable and easy to use.
- **Low Powered**: Due to the lightweight and small size it is necessary to ensure it is designed to be low powered.

## Requirements
- **Hardware**: 
  - Robotic printer chassis
  - Uses HP C6602 print head
  - Stepper motor to control positioning
- **Software**: 
  - Android app (built using Flutter and VSCode)
  - MCU main program (ESP32 board using ArduinoIDE)

## Getting Started
To get started with the project:

1. Clone this repository:
   ```bash
   git clone https://github.com/fholman/RoboticPrinter
   cd RoboticPrinter
