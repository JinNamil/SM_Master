# BLE Master Project using STM MCU

This project implements a BLE (Bluetooth Low Energy) Master using an STM microcontroller. The STM MCU acts as the central device, scanning for and connecting to BLE peripherals. This project is ideal for IoT applications where the MCU needs to communicate with various BLE devices such as sensors, beacons, or other peripherals.

## Repository

The source code for this project is hosted on GitHub: [SM_Master](https://github.com/JinNamil/SM_Master)

## Project Overview

The STM MCU serves as the BLE master, capable of connecting to multiple BLE devices (peripherals) and exchanging data with them. This project is designed to demonstrate core BLE master functionalities such as scanning, connecting, pairing, and communicating over GATT (Generic Attribute Profile).

### Key Features
- **BLE Scanning:** The BLE Master scans for available BLE devices in the vicinity.
- **Connection Management:** The BLE Master can connect to multiple peripherals and manage these connections.
- **GATT Communication:** The BLE Master can read and write data to/from peripherals through GATT services and characteristics.
- **Event-Driven Design:** The project uses an interrupt-based approach to handle BLE events for efficient real-time communication.
- **Low Power Consumption:** The project leverages STM MCU's low-power features to optimize energy usage during BLE operations.

## Requirements

### Hardware:
- STM32 MCU (e.g., STM32L4, STM32WB, or similar models with BLE capabilities).
- BLE module or integrated BLE functionality (if using STM32WB or other STM MCUs with built-in BLE).
- USB programmer (ST-LINK or similar) for flashing the firmware.
- BLE-capable peripherals (e.g., BLE sensors, beacons).

### Software:
- [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) or [Keil MDK](https://www.keil.com) for development and debugging.
- STM32CubeMX for code generation and peripheral configuration.
- STM BLE libraries and middleware.

## Installation and Setup

1. **Clone the Repository:**

    ```bash
    git clone https://github.com/JinNamil/SM_Master.git
    ```

2. **Install STM32CubeIDE:**
   Download and install [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) from the STMicroelectronics website.

3. **Open the Project in STM32CubeIDE:**
   Open STM32CubeIDE and import the project by selecting `File -> Open Projects from File System...` and navigating to the project directory.

4. **Configure the MCU:**
   Open STM32CubeMX inside the IDE to configure the BLE and other peripherals required by your project (e.g., GPIO, USART for debugging, etc.).

5. **Build and Flash the Firmware:**
   Connect your STM MCU board to your PC using a USB programmer (e.g., ST-LINK). Build the project and flash the firmware to the MCU using `Run -> Debug` or `Run -> Run` in STM32CubeIDE.

6. **Start BLE Scanning:**
   Once the firmware is running, the STM MCU will begin scanning for nearby BLE peripherals and display the available devices.

## Usage

### BLE Master Operations

1. **Scan for BLE Peripherals:** The MCU continuously scans for nearby BLE peripherals, and information about detected devices is displayed over a serial interface (e.g., UART).
   
2. **Connect to BLE Peripheral:** After scanning, the BLE Master will automatically attempt to connect to a pre-configured peripheral or can be manually set to connect to a specific device by its address.
   
3. **GATT Communication:** Once connected, the BLE Master can read and write to specific GATT services and characteristics. This can be used to control peripherals or read sensor data.

4. **Multiple Connections:** The BLE Master is capable of maintaining multiple BLE connections at once, allowing for communication with several peripherals simultaneously.

### Customization

- **Modify GATT Services:** You can update the code to communicate with specific GATT services and characteristics depending on the peripherals in use.
- **Add BLE Security Features:** Implement BLE pairing, bonding, and security features such as encrypted communication to enhance security.
- **Power Management:** Optimize the power settings by adjusting sleep modes when the BLE Master is idle.

## Example Applications

- **BLE Sensor Network:** The BLE Master can connect to multiple sensors in a smart home or industrial setting, collecting data and sending it to a central server.
- **BLE Remote Control:** Control BLE peripherals like lights or actuators from the BLE Master.
- **Health Monitoring:** The BLE Master can connect to BLE health devices (e.g., heart rate monitors) and log health data for analysis.

## Contribution

Contributions to this project are welcome. Here’s how you can contribute:

1. **Fork the repository.**
2. **Create a branch for your feature:** (`git checkout -b feature-name`)
3. **Commit your changes:** (`git commit -m 'Add feature'`)
4. **Push to the branch:** (`git push origin feature-name`)
5. **Submit a pull request.**

## Troubleshooting

- **BLE Scan Not Working:** Ensure that the BLE peripheral clock and GPIOs are properly configured in STM32CubeMX. Check that the BLE module or functionality is initialized correctly.
- **Connection Fails:** Verify that the BLE peripheral is advertising, and the device address matches the expected one.
- **Communication Issues:** Ensure that the GATT characteristics are correctly configured and that both devices (master and peripheral) support the same BLE profile.

## Future Enhancements

- **BLE Bonding and Encryption:** Implement secure pairing and bonding for better data security between the BLE Master and peripherals.
- **OTA (Over-The-Air) Firmware Updates:** Add support for updating the BLE Master's firmware via BLE without physical access to the MCU.
- **Mobile App Integration:** Develop a mobile app that can act as a BLE peripheral or central, allowing for more interactive testing.

## License

This project is licensed under the MIT License. For more details, see the [LICENSE](LICENSE) file.
