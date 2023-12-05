# Green-Oasis - IoT Irrigation System

## Note
This project is currently under development and, therefore, incomplete.

## Synopsis

### Motivation
Two main motivations drive the Green Oasis project. Firstly, it shall provide a solution for individuals, such as my parents, to go on holiday without worrying about the well-being of their garden. This IoT irrigation system aims to offer an intelligent and autonomous approach to garden maintenance, leveraging real-time weather and forecast data to ensure optimal watering even when homeowners are away. Secondly, the project reflects my interest in building IoT solutions using Qt and C++. It seeks to employ modern technologies to create an efficient and scalable irrigation system that seamlessly integrates cloud computing (Azure IoT Hub) with IoT devices (Raspberry Pi and sensor/actor clients).

### Functionality
- **Real-time Monitoring and Control:** Monitor and control irrigation devices in real-time through a user-friendly 7" touchscreen interface connected to the Raspberry Pi4 control hub.
- **Azure IoT Hub:** Facilitate secure communication between the control hub and sensor/actor clients, utilising Device Twins for efficient device configuration and management.
- **Dynamic Adjustment of Watering Schedules:** Optimise watering schedules based on real-time weather and forecast data. Moreover, adjust water time up and down based on local temperature and humidity.
- **Rainfall Detection:** Intelligently stop watering during rainfall to conserve water and promote environmental sustainability.
- **Separate Irrigation Zones:** Supports the management of multiple irrigation zones independently, allowing for customised watering schedules for different garden areas.
- **SQL Database Integration:** Utilises MySQL for persistent storage, including historical changes to device configurations, such as irrigation schedules, and significant system logs, such as errors or warnings. Those logs can assist in troubleshooting and system diagnostics

## Technology Stack

- **Programming Languages:** C++17, Python
- **Frameworks:** Qt6.6 (C++/QML)
- **IoT Services:** Azure IoT Hub, Device Twins
- **Build System:** CMake
- **Database:** MySQL
- **Version Control:** Git
- **Containerization:** Docker
- **Operating System:** Linux

## System Overview

The Green-Oasis-Pi IoT irrigation system consists of the following components:

1. **Raspberry Pi Control Hub:**
   - Developed using C++17, Qt6.6, and QML.
   - Provides a touchscreen user interface for monitoring and controlling irrigation devices.
   - Communicates with sensor/actor clients via Azure IoT Hub.

2. **Sensor/Actor Clients:**
   - Arm Cortex-M4 microcontroller-driven devices.
   - Simulated sensor/actor clients using Python and Docker containers during development.
   - Real-time monitoring of soil conditions and control of irrigation equipment.

3. **Azure IoT Hub:**
   - Facilitates secure communication between the control hub and sensor/actor clients.
   - Leverages Device Twins for storing, retrieving, and updating device metadata and state information.

4. **Weather Integration:**
   - Utilises real-time weather and forecast data to optimize watering schedules.
   - Minimizes water consumption based on environmental conditions.

5. **SQL Database:**
   - Utilizes MySQL for storing and retrieving persistent data related to the IoT irrigation system.
   - Used to log significant system-wide events, errors, or warnings. Those logs can assist in troubleshooting and system diagnostics
