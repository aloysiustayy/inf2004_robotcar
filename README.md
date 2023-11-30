# INF2004 Robot Car

## Individual Driver
- All the individual components C files are located in the `driver` folder.
- Black box testing had been conducted and documented in the folder as well.

## Integration Folder
- All the integrated files are located in the following [directory](https://github.com/aloysiustayy/inf2004_robotcar/tree/main/Robot/pico_w/wifi/freertos/irr) 

## Equipment
- A computer has the Pico C/C++ SDK installed
- Micro-USB Cable
- Raspberry Pi Pico w
- 2 x IR based Wheel Encoder
- L298N Motor Module
- 2 x DC Motors
- Ultrosonic Distance Sensor (HC-SR04)
- 3 x MH Infrared Line Tracking Sensor Module

### SETTING UP PICO SDK
Download using [this](https://github.com/raspberrypi/pico-setup-windows/releases/latest/download/pico-setup-windows-x64-standalone.exe) tool if using Windows OS.

Visual Studio Code will ask if you want to configure the pico-examples project when it is first opened; click Yes on that prompt to proceed. You will then be prompted to select a kit -- select the Pico ARM GCC - Pico SDK Toolchain with GCC arm-none-eabi entry.

### Pico W Pin Usage For Our Project
![Pico W Pinout](https://github.com/aloysiustayy/inf2004_robotcar/blob/main/pico-pinout.svg?raw=true)
- Pin 1   - Motor (PWM)
- Pin 2   - Motor (PWM)
- Pin 3   - Wheel Encoder (GND)
- Pin 4   - Wheel Encoder (Output)
- Pin 5   - Wheel Encoder (Output)
- Pin 6   - Magnetometer
- Pin 7   - Magnetometer
- Pin 8   - Wheel Encoder (GND)
- Pin 9   - Ultrasonic Trig
- Pin 10  - Ultrasonic Echo
- Pin 13  - Magnetometer (GND)
- Pin 14  - Motor (Output)
- Pin 15  - Motor (Output)
- Pin 18  - Ultrasonic (GND)
- Pin 19  - IR Sensor (Left VCC)
- Pin 20  - IR Sensor (Left Analog)
- Pin 21  - IR Sensor (Right VCC)
- Pin 22  - IR Sensor (Right Analog)
- Pin 23  - IR Sensor (GND)
- Pin 25  - IR Sensor (barcode VCC)
- Pin 26  - Motor (Output)
- Pin 27  - Motor (Output)
- Pin 28  - IR Sensor (GND)
- Pin 29  - IR Sensor 
- Pin 31  - IR Sensor (barcode)
- Pin 32  - IR Sensor 
- Pin 33  - IR Sensor (GND)
- Pin 34  - IR Sensor
- Pin 36  - Ultrasonic VCC
- Pin 38  - Motor (GND)

## How to Run
1. Navigate to the Integration Folder (`Robot/pico w/wifi/freertos/irr`).
2. Compile the `irr_sensor_sys.elf` using CMake.
3. Flash the generated `uf2` file into Pico W with all the sensor pins attached.
4. Test it out on a maze.

## Mapping Algorithm (BFS)
1. Run `MappingAlgorithm.c` using GCC.
2. Look at Serial Monitor for output.
3. Edit `variable: mazeOnGround` to update maze accordingly.

## Flow Diagram
![alt text](https://github.com/aloysiustayy/inf2004_robotcar/blob/main/FlowChart.jpeg?raw=true)

## Block Diagram
![alt text](https://github.com/aloysiustayy/inf2004_robotcar/blob/main/BlockDiagram.jpeg?raw=true)
